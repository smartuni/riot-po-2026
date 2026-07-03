import { useState } from 'react';
import { useAppSelector } from '../../../app/store';
import { useGetRootKeyQuery, useUploadRootKeyMutation } from '../../../app/store/api/api';

const truncateKey = (key, visible = 24) => {
  if (!key) return '';
  return key.length <= visible ? key : `${key.slice(0, visible)}…`;
};

const KeyDisplayBox = ({ label, value, actionLabel, onAction }) => (
  <div className="form-group" style={{ marginBottom: '0' }}>
    <label className="form-label">{label}</label>
    <div
      style={{
        display: 'flex',
        alignItems: 'center',
        gap: '8px',
        padding: '8px 10px',
        background: 'var(--bg-sidebar)',
        border: '1px solid var(--border)',
        borderRadius: '8px',
      }}
    >
      <code
        className="mono"
        style={{
          fontSize: '12px',
          color: 'var(--text-secondary)',
          flex: 1,
          overflow: 'hidden',
          textOverflow: 'ellipsis',
          whiteSpace: 'nowrap',
        }}
      >
        {value}
      </code>
      <button
        type="button"
        className="btn btn-ghost"
        style={{ padding: '4px 10px', fontSize: '11px', flexShrink: 0 }}
        onClick={onAction}
      >
        {actionLabel}
      </button>
    </div>
  </div>
);

const RootKeySection = () => {
  const user = useAppSelector((state) => state.auth.user);
  const isController = user?.role === 'controller';

  const { data: rootKey, error, isLoading } = useGetRootKeyQuery();
  const [uploadRootKey] = useUploadRootKeyMutation();

  const [showPrivateKey, setShowPrivateKey] = useState(false);
  const [uploadForm, setUploadForm] = useState({ publicKey: '', privateKey: '' });
  const [uploadStatus, setUploadStatus] = useState('idle');
  const [uploadMessage, setUploadMessage] = useState('');
  const [copiedField, setCopiedField] = useState(null);

  const isNotConfigured = error?.status === 404;
  const isConfigured = !error && rootKey;

  const handleCopy = (value, field) => {
    navigator.clipboard.writeText(value).then(() => {
      setCopiedField(field);
      setTimeout(() => setCopiedField(null), 2000);
    });
  };

  const handleUpload = async () => {
    setUploadStatus('saving');
    try {
      await uploadRootKey({
        publicKey: uploadForm.publicKey.trim(),
        privateKey: uploadForm.privateKey.trim(),
      }).unwrap();
      setUploadStatus('success');
      setUploadMessage('Root key uploaded successfully.');
      setUploadForm({ publicKey: '', privateKey: '' });
      setTimeout(() => setUploadStatus('idle'), 3000);
    } catch (err) {
      setUploadStatus('error');
      setUploadMessage('Upload failed. Please try again.');
      setTimeout(() => setUploadStatus('idle'), 5000);
    }
  };

  if (isLoading) {
    return (
      <div className="card" style={{ padding: '48px', textAlign: 'center', color: 'var(--text-secondary)' }}>
        Loading root key…
      </div>
    );
  }

  return (
    <div className="card">
      {/* Card header with title + status badge */}
      <div className="card-header">
        <span className="card-title">Root Key Pair</span>
        <span
          className={`status-badge ${isConfigured ? 'status-closed' : 'status-oos'}`}
        >
          <span className="status-dot" />
          {isConfigured ? 'Configured' : 'Not Set'}
        </span>
      </div>

      {/* Body */}
      <div style={{ padding: '20px' }}>
        {/* Read-only display when configured */}
        {isConfigured && (
          <div style={{ display: 'flex', flexDirection: 'column', gap: '16px', marginBottom: isController ? '20px' : '0' }}>
            <KeyDisplayBox
              label="Public Key"
              value={truncateKey(rootKey.publicKey)}
              actionLabel={copiedField === 'public' ? 'Copied!' : 'Copy'}
              onAction={() => handleCopy(rootKey.publicKey, 'public')}
            />
            <KeyDisplayBox
              label="Private Key"
              value={showPrivateKey ? rootKey.privateKey : '••••••••'}
              actionLabel={showPrivateKey ? 'Hide' : 'Show'}
              onAction={() => setShowPrivateKey((prev) => !prev)}
            />
          </div>
        )}

        {/* Upload form — controller only */}
        {isController && (
          <>
            {isConfigured && (
              <div
                style={{
                  borderTop: '1px solid var(--border)',
                  paddingTop: '20px',
                  marginBottom: '16px',
                }}
              >
                <span className="form-label" style={{ display: 'block', marginBottom: '12px' }}>
                  Upload New Key Pair
                </span>
              </div>
            )}
            <div className="form-group">
              <label className="form-label" htmlFor="rootkey-public">Public Key (Base64)</label>
              <textarea
                id="rootkey-public"
                className="form-input"
                rows={3}
                placeholder="Paste public key…"
                value={uploadForm.publicKey}
                onChange={(e) => setUploadForm({ ...uploadForm, publicKey: e.target.value })}
                style={{ fontFamily: 'monospace', fontSize: '12px', resize: 'vertical' }}
              />
            </div>
            <div className="form-group">
              <label className="form-label" htmlFor="rootkey-private">Private Key (Base64)</label>
              <textarea
                id="rootkey-private"
                className="form-input"
                rows={3}
                placeholder="Paste private key…"
                value={uploadForm.privateKey}
                onChange={(e) => setUploadForm({ ...uploadForm, privateKey: e.target.value })}
                style={{ fontFamily: 'monospace', fontSize: '12px', resize: 'vertical' }}
              />
            </div>

            {uploadStatus === 'success' && (
              <div style={{ color: 'var(--green-600)', marginBottom: '8px', fontSize: '13px' }}>{uploadMessage}</div>
            )}
            {uploadStatus === 'error' && (
              <div style={{ color: 'var(--red-600)', marginBottom: '8px', fontSize: '13px' }}>{uploadMessage}</div>
            )}

            <button
              className="btn btn-primary"
              style={{ width: '100%' }}
              disabled={!uploadForm.publicKey.trim() || !uploadForm.privateKey.trim() || uploadStatus === 'saving'}
              onClick={handleUpload}
            >
              {uploadStatus === 'saving' ? 'Uploading…' : 'Upload Key Pair'}
            </button>
          </>
        )}
      </div>
    </div>
  );
};

export default RootKeySection;
