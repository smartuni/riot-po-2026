import React, {useEffect, useState} from 'react';
import '../styles/RecentActivity.css';
import {fetchActivities} from "../services/api";


function RecentActivity() {
    const [activities, setActivities] = useState([]);

    useEffect(() => {
        const loadActivities = async () => {
            try {
                setActivities(await fetchActivities());
            } catch (error) {
                console.error('Fehler beim Laden der Aktivitäten', error);
            }
        };
        loadActivities();
    }, []);

    const activitiesRecent = activities.slice(Math.max(activities.length - 4, 0))

    return (
        <div className="recent-activity">
            <h2 className="recent-title">Recent Activity</h2>
            <ul className="activity-list">
                {activitiesRecent.map((activity, index) => (
                    <li key={activity.id} className="activity-item">
                        <span className="activity-time">{activity.lastTimeStamp}</span>
                        <span className="activity-action">{activity.message}</span>
                    </li>
                ))}
            </ul>
        </div>
    );
}

export default RecentActivity;
