import React, {useEffect, useState} from "react";
import { MapContainer, TileLayer, Marker, Popup, Polygon } from "react-leaflet";
import L from "leaflet";
import "leaflet/dist/leaflet.css";
import "../styles/MapView.css";
import FloodGatePopup from "./FloodGatePopup";
import {fetchGates} from "../services/api";

const getArrowIcon = (status) => {
    return L.divIcon({
        className: "custom-arrow-marker",
        html: `<div style="
      transform: rotate(45deg);
      width: 12px;
      height: 12px;
      background-color: ${status === "OPEN" ? "green" : "red"};
      border-radius: 3px;
      transform-origin: center;
    "></div>`,
        iconSize: [12, 12],
        iconAnchor: [6, 6],
    });
};

function MapView({ search, statusFilter }) {
    const [gates, setGates] = useState([]);

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
    }, []);


    const [selectedGate, setSelectedGate] = useState(null);

    const filteredGates = gates.filter(
        (gate) =>
            (gate.id.toString().toLowerCase().includes(search.toLowerCase()) ||
                gate.location.toLowerCase().includes(search.toLowerCase())) &&
            (statusFilter === "" || gate.status === statusFilter)
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
                            Device: {gate.deviceId}<br />
                            Last Update: {gate.lastUpdate}<br />
                            Confidence: 👤 {gate.workerConfidence ? "Yes" : "No"} / 📡 {gate.sensorConfidence ? "Yes" : "No"}
                        </Popup>
                    </Marker>
                ))}
            </MapContainer>
        </div>
    );
}

export default MapView;

//import React, { useEffect, useState } from "react";
//import { MapContainer, TileLayer, Marker, Tooltip } from "react-leaflet";
//import L from "leaflet";
//import "leaflet/dist/leaflet.css";
//import { fetchGates } from "../services/api";
//import FloodGatePopup from "./FloodGatePopup"; // Import hier
//
//function MapView({ search, statusFilter }) {
//    const [gates, setGates] = useState([]);
//    const [selectedGate, setSelectedGate] = useState(null);
//
//    useEffect(() => {
//        const loadGates = async () => {
//            try {
//                const data = await fetchGates();
//                setGates(data);
//            } catch (err) {
//                console.error("Fehler beim Laden der Gates:", err);
//            }
//        };
//
//        loadGates();
//    }, []);
//
//    const filteredGates = gates.filter(gate =>
//        (gate.id?.toString().includes(search) || gate.location?.toLowerCase().includes(search.toLowerCase())) &&
//        (statusFilter === "" || gate.status === statusFilter)
//    );
//
//    const getStatusColor = (status) => {
//        switch (status?.toLowerCase()) {
//            case "open": return "green";
//            case "closed": return "red";
//            default: return "gray";
//        }
//    };
//
//    const createMarkerIcon = (status) => {
//        const color = getStatusColor(status);
//        return L.divIcon({
//            html: `<div style="color:${color};font-size:2rem;">📍</div>`,
//            iconSize: [24, 24],
//            className: "custom-marker"
//        });
//    };
//
//    return (
//        <div style={{ position: "relative", height: "600px", width: "100%" }}>
//            <MapContainer center={[53.55, 9.99]} zoom={11} style={{ height: "100%", width: "100%" }}>
//                <TileLayer
//                    url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
//                    attribution="&copy; OpenStreetMap contributors"
//                />
//
//                {filteredGates.map(gate => (
//                    <Marker
//                        key={gate.id}
//                        position={[gate.latitude, gate.longitude]}
//                        icon={createMarkerIcon(gate.status)}
//                        eventHandlers={{
//                            click: () => setSelectedGate(gate),
//                        }}
//                    >
//                        <Tooltip>{gate.id}</Tooltip>
//                    </Marker>
//                ))}
//            </MapContainer>
//
//            {/* ⬇ Hier ist das Popup – außerhalb von MapContainer! */}
//            {selectedGate && (
//                <FloodGatePopup
//                    gate={selectedGate}
//                    onClose={() => setSelectedGate(null)}
//                    onStatusChange={() => {
//                        // Optional: hier könnte fetchGates neu aufgerufen werden
//                    }}
//                />
//            )}
//        </div>
//    );
//}
//
//export default MapView;

