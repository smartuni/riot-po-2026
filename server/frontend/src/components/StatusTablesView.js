import React, {useEffect, useState} from "react";
import {fetchActivities, fetchGates} from "../services/api";
import {
    TextField,
    MenuItem,
    Tabs,
    Tab,
    Tooltip,
} from "@mui/material";
import LockOpenIcon from '@mui/icons-material/LockOpen';
import LockIcon from '@mui/icons-material/Lock';
import CircleIcon from '@mui/icons-material/Circle';
import MapView from "./MapView";
import HelpOutlineIcon from '@mui/icons-material/HelpOutline';

function StatusTablesView() {
    /**
     * State-Variablen für die Gates, Suchanfrage, Filter, Ansicht und erweiterten Gate-ID.
     */
    const [gates, setGates] = useState([]);
    const [search, setSearch] = useState("");
    const [filter, setFilter] = useState("");
    const [view, setView] = useState("list");
    const [expandedGateId, setExpandedGateId] = useState(null);
    const [activities, setActivities] = useState([]);

    /**
     * Lädt die Gates beim ersten Rendern der Komponente.
     */
    useEffect(() => {
        const loadGates = async () => {
            try {
                const data = await fetchGates();
                setGates(data);
            } catch (error) {
                console.error('Fehler beim Laden der Gates', error);
            }
        };
        loadGates();
        const intervalId = setInterval(() => {
            loadGates();
        }, 300);

        return () => clearInterval(intervalId);
    }, []);

    useEffect(() => {
        const loadActivities = async () => {
            try {
                const data = await fetchActivities();
                setActivities(data);
            } catch (error) {
                console.error('Fehler beim Laden der Aktivitäten', error);
            }
        };
        loadActivities();
    }, []);

    /**
     * Rendern des angeforderten Status für die Gates.
     * @param status
     * @returns {Element}
     */
    const renderRequestedStatus = (status) => {
        switch (status) {
            case "REQUESTED_OPEN":
                return <><LockOpenIcon fontSize="small"/> OPEN</>;
            case "REQUESTED_CLOSE":
                return <><LockIcon fontSize="small"/> CLOSE</>;
            default:
                return <><CircleIcon fontSize="small"/> NONE</>;
        }
    };

    /**
     * Filtert die Gates basierend auf der Suchanfrage und dem Statusfilter.
     * @type {Array}
     */
    const filteredGates = gates.filter(gate =>
        (gate.id.toString().includes(search) || gate.location.toLowerCase().includes(search.toLowerCase())) &&
        (
            filter === "" ||
            gate.status === filter ||
            (gate.requestedStatus && gate.requestedStatus.toLowerCase().includes(filter.toLowerCase()))
        )
    );

    return (
        <div className="gate-status-container">
            <div className="gate-status-header">
                <h2>Flood Gates</h2>
                <div className="gate-controls">
                    <TextField
                        size="small"
                        placeholder="Search gates..."
                        value={search}
                        onChange={(e) => setSearch(e.target.value)}
                        style={{marginRight: "1rem"}}
                    />
                    <TextField
                        size="small"
                        select
                        value={filter}
                        onChange={(e) => setFilter(e.target.value)}
                    >
                        <MenuItem value="">All Status</MenuItem>
                        <MenuItem value="OPENED">Open</MenuItem>
                        <MenuItem value="CLOSED">Closed</MenuItem>
                        <MenuItem value="REQUESTED_CLOSE">Requested Close</MenuItem>
                        <MenuItem value="REQUESTED_OPEN">Requested Open</MenuItem>
                        <MenuItem value="REQUESTED_NONE">No Requested Status</MenuItem>
                    </TextField>
                </div>
            </div>

            <Tabs
                value={view}
                onChange={(e, newValue) => setView(newValue)}
                style={{marginTop: "1rem", marginBottom: "1rem"}}
            >
                <Tab label="List View" value="list"/>
                <Tab label="Map View" value="map"/>
            </Tabs>

            {view === "list" ? (
                <>
                    <table className="status-table">
                        <thead>
                        <tr>
                            <th>Gate ID</th>
                            <th>Location</th>
                            <th>Status</th>
                            <th>Last Update</th>
                            <th>Confidence</th>
                            {/* <th>Actions</th> */}
                        </tr>
                        </thead>
                        <tbody>
                        {filteredGates.map((gate) => (
                            <React.Fragment key={gate.id}>
                                <tr
                                    onClick={() => setExpandedGateId(expandedGateId === gate.id ? null : gate.id)}
                                    style={{cursor: "pointer"}}
                                >
                                    <td>{gate.id}</td>
                                    <td>
                                        {gate.location}<br/>
                                        <span className="coords">{gate.latitude}, {gate.longitude}</span>
                                    </td>
                                    <td>
          <span className={`badge ${gate.status.toLowerCase()}`}>
            {gate.status === "OPENED"
                ? <LockOpenIcon fontSize="small"/>
                : <LockIcon fontSize="small"/>
            } {gate.status}
          </span>
                                    </td>
                                    <td>
                                        <div>about 1 hour ago</div>
                                        <div className="date">{gate.lastUpdate}</div>
                                    </td>
                                    <td>
                                        100%<br/>
                                        <Tooltip
                                            title="Confidence reflects agreement between sensor and worker. 100% means both match.">
                                            <HelpOutlineIcon
                                                fontSize="small"
                                                className="help-icon"
                                                style={{
                                                    marginLeft: 4,
                                                    cursor: "help",
                                                    verticalAlign: "middle",
                                                    color: "#888"
                                                }}
                                            />
                                        </Tooltip>
                                    </td>
                                </tr>

                                {expandedGateId === gate.id && (
                                    <tr className="expanded-row">
                                        <td colSpan={8} style={{backgroundColor: "#f9f9f9"}}>
                                            <div>
                                                <strong>Activities</strong>
                                                {activities
                                                    .filter(activity => activity.gateId === gate.id)
                                                    .slice(-4) // Optional: nur die letzten 4 zeigen
                                                    .map((activity, index) => (
                                                        <p key={activity.id}>
                                                            <strong>{activity.lastTimeStamp}:</strong> {activity.message}
                                                        </p>
                                                    ))}
                                                {activities.filter(a => a.gateId === gate.id).length === 0 && (
                                                    <p>No activities available for this gate.</p>
                                                )}
                                            </div>
                                        </td>
                                    </tr>
                                )}
                            </React.Fragment>
                        ))}
                        </tbody>
                    </table>
                </>
            ) : (
                <MapView search={search} statusFilter={filter}/>
            )}
        </div>
    );
}

export default StatusTablesView;
