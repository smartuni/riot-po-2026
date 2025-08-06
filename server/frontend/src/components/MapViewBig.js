import React, { useState } from "react";
import { MapContainer, TileLayer, Polygon } from "react-leaflet";
import "leaflet/dist/leaflet.css";
import "../styles/MapViewBig.css";
import FloodGatePopup from "./FloodGatePopup";

// Beispiel-Gates mit Statusdaten
const safeZones = [
    {
        id: 1,
        name: "St. Pauli Floodgate",
        location: "St.Pauli",
        confidence: 100,
        status: "Open",
        coordinates: [
            [53.5452, 9.9655],
            [53.5452, 9.9675],
            [53.5448, 9.9675],
            [53.5448, 9.9655],
        ],
    },
    {
        id: 2,
        name: "HafenCity Floodgate",
        location: "HafenCity",
        confidence: 98,
        status: "Closed",
        coordinates: [
            [53.5429, 9.999],
            [53.5429, 10.001],
            [53.5425, 10.001],
            [53.5425, 9.999],
        ],
    },
    {
        id: 3,
        name: "Binnenhafen Floodgate",
        location: "Binnenhafen",
        confidence: 97,
        status: "Open",
        coordinates: [
            [53.5467, 9.9905],
            [53.5467, 9.9925],
            [53.5463, 9.9925],
            [53.5463, 9.9905],
        ],
    },
];

function MapViewBig() {
    const [selectedGate, setSelectedGate] = useState(null);

    return (
        <div className="map-view-big" style={{ position: "relative" }}>
            <h2>Mapview:</h2>
            <MapContainer
                center={[53.546, 9.99]}
                zoom={14}
                scrollWheelZoom={true}
                className="map-container"
            >
                <TileLayer
                    attribution='&copy; OpenStreetMap contributors'
                    url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
                />
                {safeZones.map((zone) => (
                    <Polygon
                        key={zone.id}
                        pathOptions={{ color: "green", fillColor: "green", fillOpacity: 0.5 }}
                        positions={zone.coordinates}
                        eventHandlers={{
                            click: () => {
                                setSelectedGate(zone);
                            },
                        }}
                    />
                ))}
            </MapContainer>

            {selectedGate && (
                <FloodGatePopup
                    gate={selectedGate}
                    onClose={() => setSelectedGate(null)}
                />
            )}
        </div>
    );
}

export default MapViewBig;