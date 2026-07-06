import React from "react";
import { MapContainer, TileLayer, Marker, Popup } from "react-leaflet";
import L from "leaflet";
import "leaflet/dist/leaflet.css";
import { useGetGatesQuery } from "../../../app/store/api/api";

const getArrowIcon = (status) => {
    const statusClass = status === "OPEN" ? "open" : status === "OUT_OF_SERVICE" ? "oos" : "";
    return L.divIcon({
        className: `map-pin ${statusClass}`.trim(),
        iconSize: [28, 28],
        iconAnchor: [14, 14],
    });
};

function MapView({ search = '', statusFilter = { Closed: true, Open: true, OOS: true } }) {
    const { data: gates = [], isLoading, error } = useGetGatesQuery();

    if (isLoading) {
        return (
            <div className="map-view" style={{ display: 'flex', justifyContent: 'center', alignItems: 'center', minHeight: '500px' }}>
                Loading map…
            </div>
        );
    }

    if (error) {
        return (
            <div className="map-view" style={{ display: 'flex', justifyContent: 'center', alignItems: 'center', minHeight: '500px', backgroundColor: 'var(--red-100)', color: 'var(--red-600)' }}>
                Failed to load gates data. Please try again later.
            </div>
        );
    }

    const statusToKey = { OPEN: 'Open', CLOSED: 'Closed', OUT_OF_SERVICE: 'OOS' };

    const filteredGates = gates.filter((gate) => {
        let matchesStatus;
        if (typeof statusFilter === 'string') {
            // String filter: "" = show all, else exact match on status or requestedStatus
            matchesStatus = statusFilter === '' || gate.status === statusFilter || gate.requestedStatus === statusFilter;
        } else {
            // Object filter: { Closed: bool, Open: bool, OOS: bool }
            const key = statusToKey[gate.status];
            matchesStatus = statusFilter[key] !== false;
        }
        const matchesSearch = search === '' ||
            gate.id.toString().toLowerCase().includes(search.toLowerCase()) ||
            (gate.location?.toLowerCase() || '').includes(search.toLowerCase());
        return matchesStatus && matchesSearch;
    });

    const getStatusStyle = (status) => {
        if (status === "OPEN") return { color: "var(--red-600)" };
        if (status === "OUT_OF_SERVICE") return { color: "var(--amber-600)" };
        return { color: "var(--green-600)" };
    };

    return (
        <div className="map-view" style={{ height: "100%", width: "100%" }}>
            <MapContainer
                center={[53.546, 9.99]}
                zoom={13}
                scrollWheelZoom={true}
                className="map-container"
                style={{ height: "100%", width: "100%" }}
            >
                <TileLayer
                    attribution='&copy; OpenStreetMap contributors'
                    url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
                />
                {filteredGates.filter(gate => gate.latitude != null && gate.longitude != null).map((gate) => (
                    <Marker
                        key={gate.id}
                        position={[gate.latitude, gate.longitude]}
                        icon={getArrowIcon(gate.status)}
                    >
                        <Popup>
                            <strong>{gate.location}</strong><br />
                            Status: <span style={getStatusStyle(gate.status)}>{gate.status}</span><br />
                            Gate-ID: <span style={{ fontFamily: "'JetBrains Mono', monospace" }}>{gate.id}</span><br />
                            Last Update: {gate.lastTimeStamp ? new Date(gate.lastTimeStamp).toLocaleString() : '—'}<br />
                            Confidence: {gate.confidence}
                        </Popup>
                    </Marker>
                ))}
            </MapContainer>
        </div>
    );
}

export default MapView;
