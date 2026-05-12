import React from "react";
import { MapContainer, TileLayer, Marker, Popup, Polygon } from "react-leaflet";
import L from "leaflet";
import "leaflet/dist/leaflet.css";
import "../styles/MapView.css";
import { CircularProgress, Alert } from "@mui/material";
// import FloodGatePopup from "./FloodGatePopup";
import { useGetGatesQuery } from "../../../app/store/api/api";

const getArrowIcon = (status) => {
    return L.divIcon({
        className: "custom-arrow-marker",
        html: `<div style="
      transform: rotate(45deg);
      width: 12px;
      height: 12px;
      background-color: ${status === "OPENED" ? "red" : "green" };
      border-radius: 3px;
      transform-origin: center;
    "></div>`,
        iconSize: [12, 12],
        iconAnchor: [6, 6],
    });
};

function MapView({ search, statusFilter }) {
    const { data: gates = [], isLoading, error } = useGetGatesQuery();

    if (isLoading) {
        return (
            <div className="map-view" style={{ display: 'flex', justifyContent: 'center', alignItems: 'center', minHeight: '500px' }}>
                <CircularProgress />
            </div>
        );
    }

    if (error) {
        return (
            <div className="map-view" style={{ display: 'flex', justifyContent: 'center', alignItems: 'center', minHeight: '500px' }}>
                <Alert severity="error">Failed to load gates data: {error.toString()}</Alert>
            </div>
        );
    }

    const filteredGates = gates.filter(
        (gate) =>
            (gate.id.toString().toLowerCase().includes(search.toLowerCase()) ||
                gate.location.toLowerCase().includes(search.toLowerCase())) &&
            (statusFilter === "" || gate.requestedStatus === statusFilter || gate.status === statusFilter)
    );

    return (
        <div className="map-view" style={{ height: "500px", width: "100%" }}>
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
                {filteredGates.map((gate) => (
                    <Marker
                        key={gate.id}
                        position={[gate.latitude, gate.longitude]}
                        icon={getArrowIcon(gate.status)}
                    >
                        <Popup>
                            <strong>{gate.location}</strong><br />
                            Status: {gate.status}<br />
                            Gate-ID: {gate.id}<br />
                            Last Update: {new Date(gate.lastTimeStamp).toLocaleString()}<br />
                            Confidence: {gate.confidence}
                        </Popup>
                    </Marker>
                ))}
            </MapContainer>
        </div>
    );
}

export default MapView;
