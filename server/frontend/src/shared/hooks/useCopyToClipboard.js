import { useState, useCallback } from 'react';

export function useCopyToClipboard(resetDelay = 2000) {
  const [copiedKey, setCopiedKey] = useState(null);

  const copy = useCallback((value, key) => {
    navigator.clipboard.writeText(value).then(() => {
      setCopiedKey(key);
      setTimeout(() => setCopiedKey(null), resetDelay);
    });
  }, [resetDelay]);

  return { copiedKey, copy };
}
