import React, { useState, useMemo, useCallback } from 'react';
import AppLayout from '../features/shell/components/AppLayout';
import { useAppSelector } from '../app/store';
import {
  useGetGatesQuery,
  useRequestGateStatusChangeMutation,
} from '../app/store/api/api';
import {
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  Button,
  Typography,
  Checkbox,
  Alert,
  AlertTitle,
  CircularProgress,
} from '@mui/material';
import WaterDropIcon from '@mui/icons-material/WaterDrop';
import WarningIcon from '@mui/icons-material/Warning';

/* ── helpers ─────────────────────────────────────────────── */

const RISK_LEVELS = {
  LOW:      { label: 'Low',      color: 'var(--blue-600)',  bg: 'rgba(59,130,246,0.1)' },
  MEDIUM:   { label: 'Medium',   color: 'var(--amber-600)', bg: 'rgba(245,158,11,0.1)' },
  HIGH:     { label: 'High',     color: '#f97316',          bg: 'rgba(249,115,22,0.1)' },
  CRITICAL: { label: 'Critical', color: 'var(--red-600)',   bg: 'rgba(239,68,68,0.1)' },
};

const getRiskLevel = (waterLevel) => {
  if (waterLevel > 8) return 'CRITICAL';
  if (waterLevel > 5) return 'HIGH';
  if (waterLevel > 3) return 'MEDIUM';
  return 'LOW';
};

const statusInfo = (status) => {
  switch (status) {
    case 'OPEN':   return { cls: 'status-open',   label: 'Open' };
    case 'CLOSED': return { cls: 'status-closed', label: 'Closed' };
    default:       return { cls: 'status-oos',    label: 'Out of Service' };
  }
};

/**
 * Determine the recommended action for a single gate given a water level.
 *  - CLOSED gates → "already closed"
 *  - Water level > 0 and gate OPEN → "close"
 *  - Water level > 8 (critical) → every gate including OOS → "close"
 */
const getRecommendedAction = (gate, waterLevel) => {
  if (gate.status === 'CLOSED') return 'already_closed';
  if (waterLevel > 8) return 'close';           // critical: all gates
  if (waterLevel > 0 && gate.status === 'OPEN') return 'close';
  return 'no_action';
};

const actionLabel = (action) => {
  switch (action) {
    case 'close':          return 'Close';
    case 'already_closed': return 'Already closed';
    default:               return 'No action';
  }
};

const priorityClass = (level) => {
  switch (level) {
    case 0: return 'priority-low';
    case 1: return 'priority-medium';
    case 2: return 'priority-high';
    case 3: return 'priority-critical';
    default: return 'priority-low';
  }
};

/* ══════════════════════════════════════════════════════════ */

const AutomationPage = () => {
  /* ── RTK Query + auth ── */
  const { data: gates = [], isLoading, error } = useGetGatesQuery();
  const [requestGateStatusChange] = useRequestGateStatusChangeMutation();
  const user = useAppSelector((state) => state.auth.user);

  const isController = user?.role === 'controller';
  const workerId = user?.workerId ?? null;

  /* ── local state ── */
  const [waterLevelInput, setWaterLevelInput] = useState('');
  const [waterLevel, setWaterLevel] = useState(null);
  const [inputError, setInputError] = useState('');
  const [editMode, setEditMode] = useState(false);
  const [selectedGateIds, setSelectedGateIds] = useState(new Set());
  const [confirmDialogOpen, setConfirmDialogOpen] = useState(false);
  const [submitting, setSubmitting] = useState(false);
  const [submitResult, setSubmitResult] = useState(null); // { type: 'success'|'error', message }

  /* ── derived: risk assessment per gate ── */
  const riskLevel = waterLevel != null ? getRiskLevel(waterLevel) : null;

  const gateAssessments = useMemo(() => {
    if (waterLevel == null) return [];
    return gates.map((gate) => ({
      gate,
      action: getRecommendedAction(gate, waterLevel),
    }));
  }, [gates, waterLevel]);

  const gatesNeedingClose = useMemo(
    () => gateAssessments.filter((a) => a.action === 'close'),
    [gateAssessments],
  );

  /* ── handlers ── */

  const handleCalculate = useCallback(() => {
    const parsed = parseFloat(waterLevelInput);
    if (waterLevelInput.trim() === '' || isNaN(parsed)) {
      setInputError('Please enter a valid number.');
      return;
    }
    if (parsed < 0) {
      setInputError('Water level must be a positive number.');
      return;
    }
    if (parsed > 100) {
      setInputError('Water level seems unrealistic (max 100m).');
      return;
    }
    setInputError('');
    setWaterLevel(parsed);

    // Pre-select gates that need closing when edit mode is on
    const recommendedIds = new Set(
      gates
        .filter((g) => getRecommendedAction(g, parsed) === 'close')
        .map((g) => g.id),
    );
    setSelectedGateIds(recommendedIds);
  }, [waterLevelInput, gates]);

  const handleToggleEditMode = () => {
    setEditMode((prev) => {
      const next = !prev;
      if (next && waterLevel != null) {
        // Entering edit mode: pre-select recommended gates
        const recommendedIds = new Set(
          gates
            .filter((g) => getRecommendedAction(g, waterLevel) === 'close')
            .map((g) => g.id),
        );
        setSelectedGateIds(recommendedIds);
      }
      return next;
    });
  };

  const toggleGateSelection = (gateId) => {
    setSelectedGateIds((prev) => {
      const next = new Set(prev);
      if (next.has(gateId)) next.delete(gateId);
      else next.add(gateId);
      return next;
    });
  };

  const handleProposeChanges = () => {
    setSubmitResult(null);
    setConfirmDialogOpen(true);
  };

  const handleConfirmChanges = async () => {
    if (!workerId) {
      setSubmitResult({ type: 'error', message: 'No worker ID found. Cannot request gate changes.' });
      setConfirmDialogOpen(false);
      return;
    }

    setSubmitting(true);
    setSubmitResult(null);

    const selectedGates = gates.filter((g) => selectedGateIds.has(g.id));
    const results = await Promise.allSettled(
      selectedGates.map((gate) =>
        requestGateStatusChange({
          gateId: gate.id,
          workerId,
          requestedStatus: 'REQUESTED_CLOSE',
        }).unwrap(),
      ),
    );

    const failures = results.filter((r) => r.status === 'rejected');

    if (failures.length === 0) {
      setSubmitResult({
        type: 'success',
        message: `Successfully requested close for ${selectedGates.length} gate${selectedGates.length !== 1 ? 's' : ''}.`,
      });
      setSelectedGateIds(new Set());
      setEditMode(false);
    } else {
      const succeeded = results.length - failures.length;
      setSubmitResult({
        type: 'error',
        message: `${succeeded} of ${results.length} gate change requests succeeded. ${failures.length} failed. Please try again for the failed gates.`,
      });
    }

    setSubmitting(false);
    setConfirmDialogOpen(false);
  };

  /* ── loading & error states ── */

  if (isLoading) {
    return (
      <AppLayout>
        <div className="card" style={{ padding: '48px', textAlign: 'center', color: 'var(--text-secondary)' }}>
          <CircularProgress size={28} />
          <div style={{ marginTop: 12 }}>Loading gates…</div>
        </div>
      </AppLayout>
    );
  }

  if (error) {
    return (
      <AppLayout>
        <div className="card" style={{ padding: '16px' }}>
          <Alert severity="error">
            <AlertTitle>Error</AlertTitle>
            Failed to load gates data. Please try again later.
          </Alert>
        </div>
      </AppLayout>
    );
  }

  const risk = riskLevel ? RISK_LEVELS[riskLevel] : null;

  /* gates selected for the confirmation dialog */
  const selectedGatesForDialog = gates.filter((g) => selectedGateIds.has(g.id));

  /* ══════════════════════════════════════════════════════════
     RENDER
     ══════════════════════════════════════════════════════════ */

  return (
    <AppLayout>
      {/* ── Water Level Input Card ── */}
      <div className="card" style={{ marginBottom: '24px' }}>
        <div className="card-header">
          <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
            <span style={{
              display: 'flex', alignItems: 'center', justifyContent: 'center',
              width: '36px', height: '36px', borderRadius: '8px',
              background: 'rgba(59,130,246,0.1)', color: 'var(--blue-600)',
            }}>
              <WaterDropIcon fontSize="small" />
            </span>
            <div>
              <div className="card-title">Water Level Control</div>
              <Typography variant="body2" style={{ color: 'var(--text-secondary)', fontSize: '13px' }}>
                Enter expected water level to see which gates should be closed
              </Typography>
            </div>
          </div>
        </div>

        <div style={{ padding: '20px', display: 'flex', gap: '12px', alignItems: 'flex-start', flexWrap: 'wrap' }}>
          <div style={{ display: 'flex', flexDirection: 'column', gap: '4px' }}>
            <label className="form-label" htmlFor="water-level-input">Water Level (meters)</label>
            <input
              id="water-level-input"
              className="form-input"
              type="number"
              value={waterLevelInput}
              onChange={(e) => setWaterLevelInput(e.target.value)}
              onKeyDown={(e) => { if (e.key === 'Enter') handleCalculate(); }}
              min={0}
              max={100}
              step="any"
              placeholder="e.g. 3.5"
              style={{ width: '240px', borderColor: inputError ? 'var(--red-600)' : 'var(--border)' }}
            />
            <span style={{ fontSize: '12px', color: inputError ? 'var(--red-600)' : 'var(--text-secondary)' }}>
              {inputError || 'Range: 0–10 meters (e.g. 3.5)'}
            </span>
          </div>
          <button
            className="btn btn-primary"
            onClick={handleCalculate}
            style={{ height: '40px', marginTop: '20px' }}
          >
            Calculate
          </button>

          {waterLevel != null && risk && (
            <div style={{
              display: 'flex', alignItems: 'center', gap: '10px',
              marginLeft: 'auto',
            }}>
              <span style={{ fontSize: '13px', color: 'var(--text-secondary)', fontWeight: 500 }}>
                Risk Level:
              </span>
              <span style={{
                display: 'inline-flex', alignItems: 'center', gap: '6px',
                padding: '4px 12px', borderRadius: '6px',
                fontSize: '13px', fontWeight: 600, whiteSpace: 'nowrap',
                background: risk.bg, color: risk.color,
              }}>
                <span style={{ width: '6px', height: '6px', borderRadius: '50%', background: risk.color }} />
                {risk.label}
              </span>
            </div>
          )}
        </div>

        {/* Water level context hint */}
        {waterLevel != null && (
          <div style={{ padding: '0 20px 16px', fontSize: '13px', color: 'var(--text-secondary)' }}>
            {riskLevel === 'CRITICAL' && (
              <span style={{ display: 'inline-flex', alignItems: 'center', gap: '4px', color: 'var(--red-600)' }}>
                <WarningIcon fontSize="small" /> Critical: all gates (including out-of-service) should be closed.
              </span>
            )}
            {riskLevel === 'HIGH' && (
              <span style={{ display: 'inline-flex', alignItems: 'center', gap: '4px', color: '#f97316' }}>
                <WarningIcon fontSize="small" /> High risk: all open gates need closing.
              </span>
            )}
            {riskLevel === 'MEDIUM' && (
              <span>Medium risk: open gates should be closed as a precaution.</span>
            )}
            {riskLevel === 'LOW' && (
              <span>Low risk: open gates are recommended for closure.</span>
            )}
          </div>
        )}
      </div>

      {/* ── Risk Assessment + Gate List ── */}
      {waterLevel != null && gateAssessments.length > 0 && (
        <div className="card" style={{ marginBottom: '24px' }}>
          <div className="card-header">
            <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
              <span style={{
                display: 'flex', alignItems: 'center', justifyContent: 'center',
                width: '36px', height: '36px', borderRadius: '8px',
                background: 'rgba(245,158,11,0.1)', color: 'var(--amber-600)',
              }}>
                <WarningIcon fontSize="small" />
              </span>
              <div className="card-title">
                Gate Consequence Assessment
              </div>
            </div>

            {/* Edit Mode toggle — controllers only */}
            {isController && (
              <button
                className={`btn ${editMode ? 'btn-primary' : 'btn-ghost'}`}
                onClick={handleToggleEditMode}
              >
                {editMode ? '✓ Edit Mode On' : 'Edit Mode'}
              </button>
            )}
          </div>

          {/* Viewer note */}
          {!isController && (
            <div style={{ padding: '12px 20px 0' }}>
              <Alert severity="info" icon={false}>
                You have read-only access. Changes can only be proposed by controllers.
              </Alert>
            </div>
          )}

          <div style={{ overflowX: 'auto' }}>
            <table className="gate-table">
              <thead>
                <tr>
                  {editMode && <th style={{ width: '40px' }}>Sel.</th>}
                  <th>Gate ID</th>
                  <th>Location</th>
                  <th>Current Status</th>
                  <th>Priority</th>
                  <th>Recommended Action</th>
                </tr>
              </thead>
              <tbody>
                {gateAssessments.map(({ gate, action }) => {
                  const si = statusInfo(gate.status);
                  const needsClosing = action === 'close';
                  const isSelected = selectedGateIds.has(gate.id);

                  // Row background highlight for gates that need closing
                  let rowBg = 'transparent';
                  if (needsClosing) {
                    if (riskLevel === 'CRITICAL' || riskLevel === 'HIGH') {
                      rowBg = 'rgba(239,68,68,0.06)';
                    } else {
                      rowBg = 'rgba(245,158,11,0.06)';
                    }
                  }

                  return (
                    <tr key={gate.id} style={{ background: rowBg }}>
                      {editMode && (
                        <td>
                          <Checkbox
                            checked={isSelected}
                            onChange={() => toggleGateSelection(gate.id)}
                            size="small"
                            sx={{
                              color: 'var(--slate-400)',
                              '&.Mui-checked': { color: 'var(--blue-600)' },
                            }}
                          />
                        </td>
                      )}
                      <td>
                        <span className="gate-id">G-{gate.id}</span>
                      </td>
                      <td>
                        {gate.location || '—'}
                        {gate.latitude != null && gate.longitude != null && (
                          <div className="coords">
                            {gate.latitude.toFixed(5)}, {gate.longitude.toFixed(5)}
                          </div>
                        )}
                      </td>
                      <td>
                        <span className={`status-badge ${si.cls}`}>
                          <span className="status-dot" />
                          {si.label}
                        </span>
                      </td>
                      <td>
                        <span
                          className={priorityClass(gate.priority ?? 0)}
                          style={{ display: 'inline-flex', alignItems: 'center' }}
                        >
                          <span className="priority-dot" />
                          {gate.priority ?? 0}
                        </span>
                      </td>
                      <td>
                        <span style={{
                          fontSize: '13px',
                          fontWeight: needsClosing ? 600 : 400,
                          color: needsClosing
                            ? (riskLevel === 'CRITICAL' || riskLevel === 'HIGH' ? 'var(--red-600)' : 'var(--amber-600)')
                            : 'var(--text-secondary)',
                        }}>
                          {actionLabel(action)}
                        </span>
                      </td>
                    </tr>
                  );
                })}
              </tbody>
            </table>
          </div>

          {/* Summary + Propose Changes */}
          {editMode && (
            <div style={{
              padding: '12px 20px',
              borderTop: '1px solid var(--border)',
              display: 'flex',
              alignItems: 'center',
              gap: '12px',
              flexWrap: 'wrap',
            }}>
              <span style={{ fontSize: '13px', color: 'var(--text-secondary)' }}>
                <strong style={{ color: 'var(--text)' }}>{selectedGateIds.size}</strong> gate{selectedGateIds.size !== 1 ? 's' : ''} selected for closure
              </span>
              <div style={{ flex: 1 }} />
              <button
                className="btn btn-primary"
                onClick={handleProposeChanges}
                disabled={selectedGateIds.size === 0}
              >
                Propose Changes
              </button>
            </div>
          )}
        </div>
      )}

      {/* Empty state: no gates loaded */}
      {waterLevel != null && gateAssessments.length === 0 && (
        <div className="card" style={{ padding: '32px', textAlign: 'center', color: 'var(--text-secondary)' }}>
          No gates available to assess.
        </div>
      )}

      {/* ── Submit result message ── */}
      {submitResult && (
        <Alert
          severity={submitResult.type === 'success' ? 'success' : 'error'}
          onClose={() => setSubmitResult(null)}
          style={{ marginBottom: '24px' }}
        >
          {submitResult.message}
        </Alert>
      )}

      {/* ── Confirmation Dialog ── */}
      <Dialog
        open={confirmDialogOpen}
        onClose={() => !submitting && setConfirmDialogOpen(false)}
        maxWidth="sm"
        fullWidth
      >
        <DialogTitle>
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
            <WarningIcon style={{ color: 'var(--amber-600)' }} />
            Confirm Gate Changes
          </div>
        </DialogTitle>
        <DialogContent>
          <Typography style={{ marginBottom: '16px' }}>
            The following <strong>{selectedGatesForDialog.length}</strong> gate{selectedGatesForDialog.length !== 1 ? 's' : ''} will have their status changed to <strong style={{ color: 'var(--red-600)' }}>REQUESTED_CLOSE</strong>:
          </Typography>

          <table className="gate-table" style={{ marginTop: '4px' }}>
            <thead>
              <tr>
                <th>Gate ID</th>
                <th>Location</th>
                <th>Current Status</th>
                <th>Action</th>
              </tr>
            </thead>
            <tbody>
              {selectedGatesForDialog.map((gate) => {
                const si = statusInfo(gate.status);
                return (
                  <tr key={gate.id}>
                    <td><span className="gate-id">G-{gate.id}</span></td>
                    <td>{gate.location || '—'}</td>
                    <td>
                      <span className={`status-badge ${si.cls}`}>
                        <span className="status-dot" />
                        {si.label}
                      </span>
                    </td>
                    <td>
                      <span style={{ fontSize: '13px', fontWeight: 600, color: 'var(--red-600)' }}>
                        Request Close
                      </span>
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </table>
        </DialogContent>
        <DialogActions style={{ padding: '12px 20px' }}>
          <Button
            onClick={() => setConfirmDialogOpen(false)}
            disabled={submitting}
            style={{ color: 'var(--text-secondary)' }}
          >
            Cancel
          </Button>
          <button
            className="btn btn-danger"
            onClick={handleConfirmChanges}
            disabled={submitting}
          >
            {submitting ? (
              <span style={{ display: 'inline-flex', alignItems: 'center', gap: '8px' }}>
                <CircularProgress size={14} color="inherit" /> Submitting…
              </span>
            ) : (
              'Confirm'
            )}
          </button>
        </DialogActions>
      </Dialog>
    </AppLayout>
  );
};

export default AutomationPage;
