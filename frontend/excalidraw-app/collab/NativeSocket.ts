import { invoke } from '@tauri-apps/api/core';
import { listen, UnlistenFn } from '@tauri-apps/api/event';

export type EventCallback = (...args: any[]) => void;

export class NativeSocket {
  
} 