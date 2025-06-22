import { invoke } from '@tauri-apps/api/core';
import { listen, UnlistenFn } from '@tauri-apps/api/event';
import { encode, decode } from 'cbor-x';
export type EventCallback = (...args: any[]) => void;

export default class Socket {
  public id: string | null = null;
  private listeners: Map<string, Set<EventCallback>> = new Map();
  private unlisten: UnlistenFn | null = null;

  private serialize(data: any): string {
    const cbor = encode(data);

    let binaryString = '';
    const len = cbor.byteLength;
    for (let i = 0; i < len; i++) {
      binaryString += String.fromCharCode(cbor[i]);
    }
    return window.btoa(binaryString);
  }

  private deserialize<T>(data: string): T {
    const binaryString = window.atob(data);

    const len = binaryString.length;
    const bytes = new Uint8Array(len);
    for (let i = 0; i < len; i++) {
      bytes[i] = binaryString.charCodeAt(i);
    }

    return decode(bytes) as T;
  }

  constructor() {
    // collab_event를 listen하여 이벤트 분기
    listen('collab_event', (event: { payload: string }) => {
      try {
        const { event: eventName, data: base64data } = JSON.parse(event.payload);
        if (eventName === 'init-room' && base64data) {
          this.id = this.deserialize<string>(base64data); // 서버가 내려주는 내 고유 ID 저장
        }
        const cbs = this.listeners.get(eventName);
        if (cbs) {
          cbs.forEach((cb) => {
            console.log(`[${eventName}] Received: ${this.deserialize<any>(base64data)}`);
            console.log(`[${eventName}] ${cb.toString()}`);
            const deserializedData = this.deserialize<any>(base64data);
            if (Array.isArray(deserializedData)) {
              cb(...deserializedData);
            } else {
              cb(deserializedData);
            }
          });
        }
      } catch (error) {
        console.error('CBOR 파싱 실패:', error);
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
    return invoke('collab_emit', { event, data: this.serialize(data) });
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