/**
 * Reads a cookie by name.
 * @param {string} name - The cookie name.
 * @returns {string|null} The cookie value, or null if not found.
 */
export function getCookie(name) {
  const nameEQ = name + '=';
  const ca = document.cookie.split(';');
  for (let i = 0; i < ca.length; i++) {
    let c = ca[i];
    while (c.charAt(0) === ' ') c = c.substring(1, c.length);
    if (c.indexOf(nameEQ) === 0) return c.substring(nameEQ.length, c.length);
  }
  return null;
}

/**
 * Sets a cookie with a given name and value.
 * @param {string} name - The cookie name.
 * @param {string} value - The cookie value.
 * @param {number} [days=7] - Number of days until expiry.
 */
export function setCookie(name, value, days = 7) {
  const expires = new Date(Date.now() + days * 24 * 60 * 60 * 1000).toUTCString();
  document.cookie = `${name}=${value}; expires=${expires}; path=/; SameSite=Lax`;
}

/**
 * Erases a cookie by name (sets Max-Age=0).
 * @param {string} name - The cookie name.
 */
export function eraseCookie(name) {
  document.cookie = name + '=; Max-Age=0; path=/';
}
