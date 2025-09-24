export {}; 

declare global {
  interface WebviewMessageEvent {
    data: unknown;
  }

  interface Window {
    chrome: {
      webview: {
        postMessage: (msg: any) => void;
        addEventListener: (
          type: 'message',
          listener: (ev: WebviewMessageEvent) => void
        ) => void;
        removeEventListener: (
          type: 'message',
          listener: (ev: WebviewMessageEvent) => void
        ) => void;
      };
    };
  }
}