import { useState } from 'react';
import { useAppSelector } from '../../../app/store';
import { useGetNodesQuery } from '../../../app/store/api/api';
import AddNodeDialog from './AddNodeDialog';
import DeleteNodeDialog from './DeleteNodeDialog';

const truncateKey = (key, visible = 24) => {
  if (!key) return '';
  return key.length <= visible ? key : `${key.slice(0, visible)}…`;
};

const NodeTable = () => {
  const user = useAppSelector((state) => state.auth.user);
  const isController = user?.role === 'controller';

  const { data: nodes = [], isLoading, error } = useGetNodesQuery();

  const [search, setSearch] = useState('');
  const [addDialogOpen, setAddDialogOpen] = useState(false);
  const [deleteDialogOpen, setDeleteDialogOpen] = useState(false);
  const [nodeToDelete, setNodeToDelete] = useState(null);
  const [copiedId, setCopiedId] = useState(null);

  const handleCopy = (value, nodeId) => {
    navigator.clipboard.writeText(value).then(() => {
      setCopiedId(nodeId);
      setTimeout(() => setCopiedId(null), 2000);
    });
  };

  const filteredNodes = nodes.filter((node) => {
    const term = search.toLowerCase();
    return (
      (node.name?.toLowerCase() || '').includes(term) ||
      (node.publicKey?.toLowerCase() || '').includes(term)
    );
  });

  if (isLoading) {
    return (
      <div className="card" style={{ padding: '48px', textAlign: 'center', color: 'var(--text-secondary)' }}>
        Loading…
      </div>
    );
  }

  if (error) {
    return (
      <div className="card" style={{ padding: '16px' }}>
        <div style={{ padding: '16px', background: 'var(--red-100)', borderRadius: '8px', color: 'var(--red-600)' }}>
          Failed to load nodes.
        </div>
      </div>
    );
  }

  return (
    <div className="card">
      {/* Toolbar */}
      <div className="table-toolbar">
        <h2 style={{ fontSize: '16px', fontWeight: 600, margin: 0, marginRight: '12px' }}>Nodes</h2>

        <input
          className="table-search"
          aria-label="Search nodes"
          placeholder="Search nodes…"
          value={search}
          onChange={(e) => setSearch(e.target.value)}
        />

        <div style={{ flex: 1 }} />

        {isController && (
          <button className="btn btn-primary" onClick={() => setAddDialogOpen(true)}>
            Add Node
          </button>
        )}
      </div>

      {/* Table or empty state */}
      {filteredNodes.length === 0 ? (
        <div
          style={{
            padding: '48px 24px',
            textAlign: 'center',
          }}
        >
          <div
            style={{
              width: '48px',
              height: '48px',
              margin: '0 auto 16px',
              borderRadius: '12px',
              background: 'var(--blue-50)',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              color: 'var(--blue-600)',
            }}
          >
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
              <rect x="2" y="2" width="20" height="20" rx="2" />
              <path d="M7 7h.01M12 7h5M7 12h.01M12 12h5M7 17h.01M12 17h5" />
            </svg>
          </div>
          <p style={{ fontSize: '15px', fontWeight: 600, color: 'var(--text)', marginBottom: '4px' }}>
            No nodes registered yet
          </p>
          <p style={{ fontSize: '13px', color: 'var(--text-secondary)' }}>
            {isController
              ? 'Click "Add Node" to register your first device.'
              : 'Nodes will appear here once they are registered.'}
          </p>
        </div>
      ) : (
        <table className="gate-table">
          <thead>
            <tr>
              <th>Name</th>
              <th>Public Key</th>
              <th>Created Date</th>
              {isController && <th style={{ textAlign: 'right' }}>Actions</th>}
            </tr>
          </thead>
          <tbody>
            {filteredNodes.map((node) => (
              <tr key={node.id}>
                <td data-label="Name">
                  <span className="gate-id">{node.name}</span>
                </td>
                <td data-label="Public Key">
                  <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                    <code
                      className="mono"
                      style={{
                        fontSize: '12px',
                        color: 'var(--text-secondary)',
                        overflow: 'hidden',
                        textOverflow: 'ellipsis',
                        whiteSpace: 'nowrap',
                      }}
                    >
                      {truncateKey(node.publicKey)}
                    </code>
                    <button
                      type="button"
                      className="action-link"
                      style={{ flexShrink: 0, fontSize: '12px' }}
                      onClick={() => handleCopy(node.publicKey, node.id)}
                    >
                      {copiedId === node.id ? 'Copied!' : 'Copy'}
                    </button>
                  </div>
                </td>
                <td data-label="Created Date">
                  <span className="last-update">
                    {node.createdAt ? new Date(node.createdAt).toLocaleDateString() : '—'}
                  </span>
                </td>
                {isController && (
                  <td data-label="Actions" style={{ textAlign: 'right' }}>
                    <button
                      type="button"
                      className="action-link"
                      style={{ color: 'var(--red-600)' }}
                      onClick={() => {
                        setNodeToDelete(node);
                        setDeleteDialogOpen(true);
                      }}
                    >
                      Delete
                    </button>
                  </td>
                )}
              </tr>
            ))}
          </tbody>
        </table>
      )}

      {/* Dialogs */}
      <AddNodeDialog open={addDialogOpen} onClose={() => setAddDialogOpen(false)} />
      <DeleteNodeDialog
        open={deleteDialogOpen}
        node={nodeToDelete}
        onClose={() => {
          setDeleteDialogOpen(false);
          setNodeToDelete(null);
        }}
      />
    </div>
  );
};

export default NodeTable;
