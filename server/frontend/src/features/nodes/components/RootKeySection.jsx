import { useState } from 'react';
import { useAppSelector } from '../../../app/store';
import { useGetRootKeyQuery, useUploadRootKeyMutation } from '../../../app/store/api/api';
import { truncateKey } from '../../../shared/utils/format';
import { useCopyToClipboard } from '../../../shared/hooks/useCopyToClipboard';
import LoadingCard from '../../../shared/components/LoadingCard';
import KeyDisplayBox from './KeyDisplayBox';

const RootKeySection = () => {
  const user = useAppSelector((state) => state.auth.user);
  const isController = user?.role === 'controller';

  const { data: rootKey, error, isLoading } = useGetRootKeyQuery();
  const [uploadRootKey] = useUploadRootKeyMutation();

  const [showPrivateKey, setShowPrivateKey] = useState(false);
  const [uploadForm, setUploadForm] = useState({ publicKey: '', privateKey: '' });
  const [uploadStatus, setUploadStatus] = useState('idle');
  const [uploadMessage, setUploadMessage] = useState('');
  const { copiedKey, copy } = useCopyToClipboard();

  const isConfigured = !error && rootKey;

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

  if (isLoading) return <LoadingCard label="Loading root key…" />;

  if (!isController) return null;

  return (
    <div className="card">
      <div className="card-header">
        <span className="card-title">Root Key Pair</span>
        <span className={`status-badge ${isConfigured ? 'status-closed' : 'status-oos'}`}>
          <span className="status-dot" />
          {isConfigured ? 'Configured' : 'Not Set'}
        </span>
      </div>

      <div style={{ padding: '20px' }}>
        {isConfigured && (
          <div style={{ display: 'flex', flexDirection: 'column', gap: '16px', marginBottom: isController ? '20px' : '0' }}>
            <KeyDisplayBox
              label="Public Key"
              value={truncateKey(rootKey.publicKey)}
              actionLabel={copiedKey === 'public' ? 'Copied!' : 'Copy'}
              onAction={() => copy(rootKey.publicKey, 'public')}
            />
            <KeyDisplayBox
              label="Private Key"
              value={showPrivateKey ? rootKey.privateKey : '••••••••'}
              actionLabel={showPrivateKey ? 'Hide' : 'Show'}
              onAction={() => setShowPrivateKey((prev) => !prev)}
            />
          </div>
        )}

        {isController && (
          <>
            {isConfigured && (
              <div style={{ borderTop: '1px solid var(--border)', paddingTop: '20px', marginBottom: '16px' }}>
                <span className="form-label" style={{ display: 'block', marginBottom: '12px' }}>
                  Upload New Key Pair
                </span>
              </div>
            )}
            <RootKeyUploadForm
              form={uploadForm}
              setForm={setUploadForm}
              status={uploadStatus}
              message={uploadMessage}
              onSubmit={handleUpload}
            />
          </>
        )}
      </div>
    </div>
  );
};

const RootKeyUploadForm = ({ form, setForm, status, message, onSubmit }) => (
  <>
    <div className="form-group">
      <label className="form-label" htmlFor="rootkey-public">Public Key (Base64)</label>
      <textarea
        id="rootkey-public"
        className="form-input"
        rows={3}
        placeholder="Paste public key…"
        value={form.publicKey}
        onChange={(e) => setForm({ ...form, publicKey: e.target.value })}
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
        value={form.privateKey}
        onChange={(e) => setForm({ ...form, privateKey: e.target.value })}
        style={{ fontFamily: 'monospace', fontSize: '12px', resize: 'vertical' }}
      />
    </div>

    {status === 'success' && (
      <div style={{ color: 'var(--green-600)', marginBottom: '8px', fontSize: '13px' }}>{message}</div>
    )}
    {status === 'error' && (
      <div style={{ color: 'var(--red-600)', marginBottom: '8px', fontSize: '13px' }}>{message}</div>
    )}

    <button
      className="btn btn-primary"
      style={{ width: '100%' }}
      disabled={!form.publicKey.trim() || !form.privateKey.trim() || status === 'saving'}
      onClick={onSubmit}
    >
      {status === 'saving' ? 'Uploading…' : 'Upload Key Pair'}
    </button>
  </>
);

export default RootKeySection;
