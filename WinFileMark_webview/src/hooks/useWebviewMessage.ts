import { useEffect } from 'react';

export function useWebviewMessage(
  handler: (data: unknown) => void
): void {
  useEffect(() => {

    const webview = (window as any).chrome?.webview;
    
    if (!webview || typeof webview.addEventListener !== 'function') {
      console.warn('[useWebviewMessage] WebView API not available');
      return;
    }

    const listener = (e: WebviewMessageEvent) => {
      handler(e.data);
    };

    window.chrome.webview.addEventListener('message', listener);
    return () => {
      window.chrome.webview.removeEventListener('message', listener);
    };
  }, [handler]);
}
