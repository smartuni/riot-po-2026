import { useState } from 'react';
import { Dialog, DialogTitle, DialogContent, DialogActions } from '@mui/material';
import { useAddNodeMutation } from '../../../app/store/api/api';

const AddNodeDialog = ({ open, onClose }) => {
  const [addNode] = useAddNodeMutation();
  const [formData, setFormData] = useState({ name: '', publicKey: '' });
  const [submitting, setSubmitting] = useState(false);
  const [errorMessage, setErrorMessage] = useState('');

  const isFormValid = formData.name.trim() !== '' && formData.publicKey.trim() !== '';

  const handleSubmit = async () => {
    setSubmitting(true);
    setErrorMessage('');
    try {
      await addNode({
        name: formData.name.trim(),
        publicKey: formData.publicKey.trim(),
      }).unwrap();
      setFormData({ name: '', publicKey: '' });
      onClose();
    } catch (err) {
      setErrorMessage(err?.data?.error || 'Failed to add node. Please try again.');
    } finally {
      setSubmitting(false);
    }
  };

  const handleClose = () => {
    if (!submitting) {
      setFormData({ name: '', publicKey: '' });
      setErrorMessage('');
      onClose();
    }
  };

  return (
    <Dialog open={open} onClose={handleClose}>
      <DialogTitle>Add New Node</DialogTitle>
      <DialogContent>
        <div className="form-group">
          <label className="form-label" htmlFor="add-node-name">Name</label>
          <input
            id="add-node-name"
            className="form-input"
            required
            value={formData.name}
            onChange={(e) => setFormData({ ...formData, name: e.target.value })}
          />
        </div>
        <div className="form-group">
          <label className="form-label" htmlFor="add-node-publickey">Public Key</label>
          <textarea
            id="add-node-publickey"
            className="form-input"
            rows={4}
            required
            placeholder="Paste public key…"
            value={formData.publicKey}
            onChange={(e) => setFormData({ ...formData, publicKey: e.target.value })}
          />
        </div>
        {errorMessage && (
          <div style={{ color: 'var(--red-600)', marginBottom: '8px' }}>{errorMessage}</div>
        )}
      </DialogContent>
      <DialogActions>
        <button className="btn btn-ghost" onClick={handleClose} disabled={submitting}>
          Cancel
        </button>
        <button
          className="btn btn-primary"
          disabled={!isFormValid || submitting}
          onClick={handleSubmit}
        >
          {submitting ? 'Adding…' : 'Add'}
        </button>
      </DialogActions>
    </Dialog>
  );
};

export default AddNodeDialog;
