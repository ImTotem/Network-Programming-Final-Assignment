import { invoke } from '@tauri-apps/api/core';
import { listen, UnlistenFn } from '@tauri-apps/api/event';

export type EventCallback = (...args: any[]) => void;

export default class Socket {
  public id: string | null = null;
  private listeners: Map<string, Set<EventCallback>> = new Map();
  private unlisten: UnlistenFn | null = null;

  // base64 인코딩/디코딩 유틸
  private toBase64(str: string): string {
    if (typeof window !== 'undefined' && window.btoa) {
      return window.btoa(unescape(encodeURIComponent(str)));
    } else {
      return Buffer.from(str, 'utf-8').toString('base64');
    }
  }
  private fromBase64(str: string): string {
    if (typeof window !== 'undefined' && window.atob) {
      return decodeURIComponent(escape(window.atob(str)));
    } else {
      return Buffer.from(str, 'base64').toString('utf-8');
    }
  }

  constructor() {
    // collab_event를 listen하여 이벤트 분기
    listen('collab_event', (event: { payload: [string, string] }) => {
      const [eventName, base64data] = event.payload;
      const decodedData = this.fromBase64(base64data);
      if (eventName === 'init-room' && decodedData) {
        this.id = decodedData; // 서버가 내려주는 내 고유 ID 저장
      }
      const cbs = this.listeners.get(eventName);
      if (cbs) {
        cbs.forEach(cb => cb(decodedData, eventName));
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

  emit(event: string, data: any) {
    // data를 base64 인코딩 후 event, data로 분리해서 전송
    return invoke('collab_emit', { event, data: this.toBase64(data) });
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