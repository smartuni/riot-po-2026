export const truncateKey = (key, visible = 24) => {
  if (!key) return '';
  return key.length <= visible ? key : `${key.slice(0, visible)}…`;
};
