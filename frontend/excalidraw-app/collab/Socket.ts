import { invoke } from '@tauri-apps/api/core';
import { listen, UnlistenFn } from '@tauri-apps/api/event';

export type EventCallback = (...args: any[]) => void;

export default class Socket {
  public id: string | null = null;
  private listeners: Map<string, Set<EventCallback>> = new Map();
  private unlisten: UnlistenFn | null = null;

  constructor() {
    // collab_event를 listen하여 이벤트 분기
    listen('collab_event', (event: { payload: [string, any[]] }) => {
      const [eventName, args] = event.payload;
      if (eventName === 'init-room' && args[0]) {
        this.id = args[0]; // 서버가 내려주는 내 고유 ID 저장
      }
      const cbs = this.listeners.get(eventName);
      if (cbs) {
        cbs.forEach(cb => cb(...args));
      }
    }).then(unlisten => {
      this.unlisten = unlisten;
    });
  }

  async connect(host: string, port: number) {
    return await invoke('collab_connect', { host, port });
  }

  async disconnect() {
    return await invoke('collab_disconnect');
  }

  on(event: string, cb: EventCallback) {
    if (!this.listeners.has(event)) {
      this.listeners.set(event, new Set());
    }
    this.listeners.get(event)!.add(cb);
  }

  off(event: string, cb?: EventCallback) {
    if (!this.listeners.has(event)) return;
    if (cb) {
      this.listeners.get(event)!.delete(cb);
    } else {
      this.listeners.delete(event);
    }
  }

  once(event: string, cb: EventCallback) {
    const wrapper = (...args: any[]) => {
      this.off(event, wrapper);
      cb(...args);
    };
    this.on(event, wrapper);
  }

  emit(event: string, ...args: any[]) {
    return invoke('collab_emit', { event, args });
  }

  close() {
    if (this.unlisten) {
      this.unlisten();
      this.unlisten = null;
    }
    invoke('collab_disconnect');
    this.listeners.clear();
    this.id = null;
  }
} 