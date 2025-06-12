import React from 'react';
import '../styles/RecentActivity.css';

const activities = [
    { time: "10:23", action: "Gate A geöffnet" },
    { time: "10:25", action: "Gate B geschlossen" },
    { time: "10:30", action: "Alarm ausgelöst" },
    { time: "10:45", action: "Gate C geöffnet" },
    { time: "10:45", action: "Gate C geöffnet" },
    { time: "10:45", action: "Gate C geöffnet" },
];
const activitiesRecent = activities.slice(Math.max(activities.length - 4, 0))


function RecentActivity() {
    return (
        <div className="recent-activity">
            <h2 className="recent-title">Recent Activity</h2>
            <ul className="activity-list">
                {activitiesRecent.map((item, index) => (
                    <li key={index} className="activity-item">
                        <span className="activity-time">{item.time}</span>
                        <span className="activity-action">{item.action}</span>
                    </li>
                ))}
            </ul>
        </div>
    );
}

export default RecentActivity;
