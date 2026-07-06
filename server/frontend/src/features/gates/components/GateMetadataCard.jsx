import React, { useState } from 'react';
import AddIcon from '@mui/icons-material/Add';
import EditIcon from '@mui/icons-material/Edit';
import DeleteIcon from '@mui/icons-material/Delete';
import CheckIcon from '@mui/icons-material/Check';
import CloseIcon from '@mui/icons-material/Close';
import LockIcon from '@mui/icons-material/Lock';
import WaterDropIcon from '@mui/icons-material/WaterDrop';
import {
    useGetGateMetadataQuery,
    useCreateGateMetadataMutation,
    useUpdateGateMetadataMutation,
    useDeleteGateMetadataMutation,
    useUpdateHeightAboveNNMutation,
} from '../../../app/store/api/api';

const GateMetadataCard = ({ gate, isController, embedded = false }) => {
    const { data: metadata = [], isLoading, isError } = useGetGateMetadataQuery(gate.id);
    const [createMetadata, { isLoading: isCreating }] = useCreateGateMetadataMutation();
    const [updateMetadata, { isLoading: isUpdating }] = useUpdateGateMetadataMutation();
    const [deleteMetadata, { isLoading: isDeleting }] = useDeleteGateMetadataMutation();
    const [updateHeightAboveNN, { isLoading: isUpdatingHeight }] = useUpdateHeightAboveNNMutation();

    const [editingHeight, setEditingHeight] = useState(false);
    const [heightValue, setHeightValue] = useState('');
    const [adding, setAdding] = useState(false);
    const [newKey, setNewKey] = useState('');
    const [newValue, setNewValue] = useState('');
    const [editingId, setEditingId] = useState(null);
    const [editKey, setEditKey] = useState('');
    const [editValue, setEditValue] = useState('');

    const startEditHeight = () => {
        setHeightValue(gate.heightAboveNN != null ? String(gate.heightAboveNN) : '');
        setEditingHeight(true);
    };

    const cancelEditHeight = () => {
        setEditingHeight(false);
        setHeightValue('');
    };

    const handleSaveHeight = async () => {
        const parsed = parseFloat(heightValue);
        if (isNaN(parsed)) return;
        try {
            await updateHeightAboveNN({ gateId: gate.id, heightAboveNN: parsed }).unwrap();
            cancelEditHeight();
        } catch (error) {
            console.error('Failed to update height above NN:', error);
            alert('Failed to update height above NN.');
        }
    };

    const handleAdd = async () => {
        if (!newKey.trim()) return;
        try {
            await createMetadata({ gateId: gate.id, key: newKey.trim(), value: newValue.trim() }).unwrap();
            setNewKey('');
            setNewValue('');
            setAdding(false);
        } catch (error) {
            console.error('Failed to create metadata:', error);
            alert('Failed to create metadata.');
        }
    };

    const startEdit = (item) => {
        setEditingId(item.id);
        setEditKey(item.key);
        setEditValue(item.value ?? '');
    };

    const cancelEdit = () => {
        setEditingId(null);
        setEditKey('');
        setEditValue('');
    };

    const handleSaveEdit = async () => {
        if (!editKey.trim()) return;
        try {
            await updateMetadata({
                gateId: gate.id,
                metadataId: editingId,
                key: editKey.trim(),
                value: editValue.trim(),
            }).unwrap();
            cancelEdit();
        } catch (error) {
            console.error('Failed to update metadata:', error);
            alert('Failed to update metadata.');
        }
    };

    const handleDelete = async (item) => {
        try {
            await deleteMetadata({ gateId: gate.id, metadataId: item.id }).unwrap();
        } catch (error) {
            console.error('Failed to delete metadata:', error);
            alert('Failed to delete metadata.');
        }
    };

    const content = (
        <>
            <div className="meta-highlight" style={{
                borderRadius: '10px',
                padding: '16px 20px',
                marginBottom: '24px',
            }}>
                <div style={{
                    display: 'flex',
                    alignItems: 'center',
                    gap: '6px',
                    marginBottom: '12px',
                    fontSize: '13px',
                    fontWeight: 600,
                    textTransform: 'uppercase',
                    letterSpacing: '0.05em',
                }} className="meta-highlight-text">
                    <WaterDropIcon style={{ fontSize: '16px' }} />
                    Closing Threshold
                </div>
                <div style={{
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'space-between',
                    gap: '12px',
                }}>
                    <div style={{ display: 'flex', alignItems: 'center', gap: '6px' }}>
                        <LockIcon style={{ fontSize: '14px', color: 'var(--text-secondary)' }} />
                        <span style={{ fontSize: '12px', color: 'var(--text-secondary)', fontWeight: 600 }}>
                            Height above NN
                        </span>
                    </div>
                    {editingHeight ? (
                        <div style={{ display: 'flex', alignItems: 'center', gap: '6px' }}>
                            <input
                                className="form-input"
                                type="number"
                                step="0.01"
                                value={heightValue}
                                onChange={(e) => setHeightValue(e.target.value)}
                                placeholder="0.00"
                                style={{ width: '100px', fontSize: '14px', padding: '6px 10px' }}
                                autoFocus
                                onKeyDown={(e) => {
                                    if (e.key === 'Enter') handleSaveHeight();
                                    if (e.key === 'Escape') cancelEditHeight();
                                }}
                            />
                            <span style={{ fontSize: '14px', color: 'var(--text-secondary)', fontWeight: 600 }}>m</span>
                            <button className="action-link" onClick={handleSaveHeight} style={{ color: 'var(--green-600)' }} disabled={isUpdatingHeight}>
                                <CheckIcon fontSize="small" />
                            </button>
                            <button className="action-link" onClick={cancelEditHeight} style={{ color: 'var(--text-secondary)' }}>
                                <CloseIcon fontSize="small" />
                            </button>
                        </div>
                    ) : (
                        <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                            <span className="mono meta-highlight-text" style={{ fontSize: '16px', fontWeight: 600 }}>
                                {gate.heightAboveNN != null ? `${gate.heightAboveNN} m` : '—'}
                            </span>
                            {isController && (
                                <button className="action-link" onClick={startEditHeight} style={{ color: 'var(--blue-600)' }}>
                                    <EditIcon style={{ fontSize: '16px' }} />
                                </button>
                            )}
                        </div>
                    )}
                </div>
            </div>

            <div style={{
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'space-between',
                marginBottom: '12px',
            }}>
                <span style={{
                    fontSize: '13px',
                    fontWeight: 600,
                    color: 'var(--text-secondary)',
                    textTransform: 'uppercase',
                    letterSpacing: '0.05em',
                }}>
                    Additional Metadata
                </span>
                {isController && !adding && (
                    <button
                        className="btn btn-ghost"
                        style={{ fontSize: '12px', padding: '4px 10px' }}
                        onClick={() => setAdding(true)}
                    >
                        <AddIcon fontSize="small" /> Add
                    </button>
                )}
            </div>

            {isError ? (
                <div style={{ padding: '12px 0', color: 'var(--red-600)', fontSize: '13px' }}>
                    Failed to load metadata.
                </div>
            ) : isLoading ? (
                <div style={{ padding: '12px 0', color: 'var(--text-secondary)', fontSize: '13px' }}>
                    Loading...
                </div>
            ) : !adding && metadata.length === 0 ? (
                <div style={{
                    padding: '24px 0',
                    color: 'var(--text-secondary)',
                    fontSize: '13px',
                    textAlign: 'center',
                }}>
                    No additional metadata.
                </div>
            ) : null}

            {metadata.map((item) => (
                <div
                    key={item.id}
                    style={{
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'space-between',
                        padding: '12px 0',
                        borderBottom: '1px solid var(--border)',
                    }}
                >
                    {editingId === item.id ? (
                        <>
                            <div style={{ display: 'flex', gap: '8px', flex: 1, marginRight: '8px' }}>
                                <input
                                    className="form-input"
                                    value={editKey}
                                    onChange={(e) => setEditKey(e.target.value)}
                                    placeholder="Key"
                                    style={{ fontSize: '13px', padding: '6px 10px' }}
                                    autoFocus
                                    onKeyDown={(e) => {
                                        if (e.key === 'Enter') handleSaveEdit();
                                        if (e.key === 'Escape') cancelEdit();
                                    }}
                                />
                                <input
                                    className="form-input"
                                    value={editValue}
                                    onChange={(e) => setEditValue(e.target.value)}
                                    placeholder="Value"
                                    style={{ fontSize: '13px', padding: '6px 10px' }}
                                    onKeyDown={(e) => {
                                        if (e.key === 'Enter') handleSaveEdit();
                                        if (e.key === 'Escape') cancelEdit();
                                    }}
                                />
                            </div>
                            <div style={{ display: 'flex', gap: '4px' }}>
                                <button className="action-link" onClick={handleSaveEdit} style={{ color: 'var(--green-600)' }} aria-label="Save metadata edit" disabled={isUpdating}>
                                        <CheckIcon fontSize="small" />
                                    </button>
                                <button className="action-link" onClick={cancelEdit} style={{ color: 'var(--text-secondary)' }}>
                                    <CloseIcon fontSize="small" />
                                </button>
                            </div>
                        </>
                    ) : (
                        <>
                            <div style={{ display: 'flex', flexDirection: 'column', gap: '2px' }}>
                                <span style={{ fontSize: '12px', color: 'var(--text-secondary)', fontWeight: 600 }}>
                                    {item.key}
                                </span>
                                <span className="mono" style={{ fontSize: '14px', color: 'var(--text)' }}>
                                    {item.value ?? '—'}
                                </span>
                            </div>
                            {isController && (
                                <div style={{ display: 'flex', gap: '4px' }}>
                                    <button className="action-link" onClick={() => startEdit(item)} style={{ color: 'var(--blue-600)' }}>
                                        <EditIcon style={{ fontSize: '16px' }} />
                                    </button>
                                    <button className="action-link" onClick={() => handleDelete(item)} style={{ color: 'var(--red-600)' }} aria-label="Delete metadata item" disabled={isDeleting}>
                                        <DeleteIcon style={{ fontSize: '16px' }} />
                                    </button>
                                </div>
                            )}
                        </>
                    )}
                </div>
            ))}

            {adding && (
                <div style={{
                    display: 'flex',
                    alignItems: 'center',
                    gap: '8px',
                    padding: '12px 0',
                    borderBottom: '1px solid var(--border)',
                }}>
                    <input
                        className="form-input"
                        value={newKey}
                        onChange={(e) => setNewKey(e.target.value)}
                        placeholder="Key"
                        style={{ fontSize: '13px', padding: '6px 10px' }}
                        autoFocus
                        onKeyDown={(e) => {
                            if (e.key === 'Enter') handleAdd();
                            if (e.key === 'Escape') { setAdding(false); setNewKey(''); setNewValue(''); }
                        }}
                    />
                    <input
                        className="form-input"
                        value={newValue}
                        onChange={(e) => setNewValue(e.target.value)}
                        placeholder="Value"
                        style={{ fontSize: '13px', padding: '6px 10px' }}
                        onKeyDown={(e) => {
                            if (e.key === 'Enter') handleAdd();
                            if (e.key === 'Escape') { setAdding(false); setNewKey(''); setNewValue(''); }
                        }}
                    />
                    <button className="action-link" onClick={handleAdd} style={{ color: 'var(--green-600)' }} aria-label="Add metadata item" disabled={isCreating}>
                        <CheckIcon fontSize="small" />
                    </button>
                    <button className="action-link" onClick={() => { setAdding(false); setNewKey(''); setNewValue(''); }} style={{ color: 'var(--text-secondary)' }}>
                        <CloseIcon fontSize="small" />
                    </button>
                </div>
            )}
        </>
    );

    if (embedded) {
        return content;
    }

    return (
        <div className="card">
            <div className="card-header">
                <span className="card-title">Gate Metadata</span>
            </div>
            <div style={{ padding: '16px 20px' }}>
                {content}
            </div>
        </div>
    );
};

export default GateMetadataCard;
