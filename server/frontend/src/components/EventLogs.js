import React from 'react';
import '../styles/EventLogs.css';
import HeaderBar from "./HeaderBar";
import Sidebar from "./Sidebar";
import InfoBoxes from "./InfoBoxes";
import MapViewBig from "./MapViewBig";

const EventLogs = () => {
    const events = [
        {
            id: 1,
            title: "Gate 4 has been opened",
            worker: "Worker 3",
            time: "12.32 pm",
            location: "Landungsbrücken"
        },
        {
            id: 2,
            title: "Gate 2 has been closed",
            worker: "Worker 3",
            time: "12.32 pm",
            location: "Landungsbrücken"
        },
        {
            id: 3,
            title: "Gate 1 needs to be closed",
            worker: "Worker 3",
            time: "12.32 pm",
            location: "Landungsbrücken"
        },
        {
            id: 4,
            title: "Gate 3 status unknown",
            worker: "Worker 3",
            time: "12.32 pm",
            location: "Landungsbrücken"
        },
        {
            id: 5,
            title: "Gate 3 has been closed",
            worker: "Worker 3",
            time: "12.32 pm",
            location: "Landungsbrücken"
        }
    ];

    return (
        <div>
            <HeaderBar />
            <div className="container">
                <Sidebar />
                <div className="event-logs-container">
                    <h1 className="event-logs-title">EventLogs</h1>

                    <div className="event-logs-grid">
                        {events.map((event) => (
                            <div key={event.id} className="event-card">
                                <div className="event-header">
                                    <h3 className="event-title">{event.title}</h3>
                                </div>

                                <div className="event-content">
                                    <table className="event-table">
                                        <thead>
                                        <tr className="table-header-row">
                                            <th className="table-header">Worker:</th>
                                            <th className="table-header">Time:</th>
                                            <th className="table-header">Location:</th>
                                        </tr>
                                        </thead>
                                        <tbody>
                                        <tr>
                                            <td className="table-cell worker-cell">{event.worker}</td>
                                            <td className="table-cell">{event.time}</td>
                                            <td className="table-cell">{event.location}</td>
                                        </tr>
                                        </tbody>
                                    </table>
                                </div>
                            </div>
                        ))}
                    </div>
                </div>
            </div>
        </div>
    );
};

export default EventLogs;