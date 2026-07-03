import { useState } from 'react';
import { useAppSelector } from '../../../app/store';
import { useGetNodesQuery } from '../../../app/store/api/api';
import { truncateKey } from '../../../shared/utils/format';
import { useCopyToClipboard } from '../../../shared/hooks/useCopyToClipboard';
import LoadingCard from '../../../shared/components/LoadingCard';
import EmptyState from '../../../shared/components/EmptyState';
import AddNodeDialog from './AddNodeDialog';
import DeleteNodeDialog from './DeleteNodeDialog';

const NodeIcon = () => (
  <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
    <rect x="2" y="2" width="20" height="20" rx="2" />
    <path d="M7 7h.01M12 7h5M7 12h.01M12 12h5M7 17h.01M12 17h5" />
  </svg>
);

const NodeRow = ({ node, isController, copiedId, onCopy, onDelete }) => (
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
          onClick={() => onCopy(node.publicKey, node.id)}
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
          onClick={() => onDelete(node)}
        >
          Delete
        </button>
      </td>
    )}
  </tr>
);

const NodeTable = () => {
  const user = useAppSelector((state) => state.auth.user);
  const isController = user?.role === 'controller';

  const { data: nodes = [], isLoading, error } = useGetNodesQuery();

  const [search, setSearch] = useState('');
  const [addDialogOpen, setAddDialogOpen] = useState(false);
  const [deleteDialogOpen, setDeleteDialogOpen] = useState(false);
  const [nodeToDelete, setNodeToDelete] = useState(null);
  const { copiedKey, copy } = useCopyToClipboard();

  const filteredNodes = nodes.filter((node) => {
    const term = search.toLowerCase();
    return (
      (node.name?.toLowerCase() || '').includes(term) ||
      (node.publicKey?.toLowerCase() || '').includes(term)
    );
  });

  if (isLoading) return <LoadingCard label="Loading…" />;

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

      {filteredNodes.length === 0 ? (
        <EmptyState
          icon={<NodeIcon />}
          title="No nodes registered yet"
          subtitle={isController
            ? 'Click "Add Node" to register your first device.'
            : 'Nodes will appear here once they are registered.'}
        />
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
              <NodeRow
                key={node.id}
                node={node}
                isController={isController}
                copiedId={copiedKey}
                onCopy={copy}
                onDelete={(n) => {
                  setNodeToDelete(n);
                  setDeleteDialogOpen(true);
                }}
              />
            ))}
          </tbody>
        </table>
      )}

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
