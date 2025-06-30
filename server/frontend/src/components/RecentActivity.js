import React, { useEffect, useState } from 'react';
import '../styles/RecentActivity.css';
import { fetchActivities, fetchGates } from "../services/api";
import { FiClock } from 'react-icons/fi';
import SockJS from 'sockjs-client';
import { Stomp } from '@stomp/stompjs';

function RecentActivity() {
    const [activities, setActivities] = useState([]);

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

        // const intervalId = setInterval(() => {
        //     loadActivities();
        // }, 300);

        // return () => clearInterval(intervalId);
    }, []);

    useEffect(() => {
        const socket = new SockJS('http://localhost:8080/ws');
        const stompClient = Stomp.over(socket);

        stompClient.connect({}, () => {
            stompClient.subscribe('/topic/gate-activities', (message) => {
                const activity = JSON.parse(message.body);
                setActivities(prev => [...prev, activity]);
            });

            stompClient.subscribe('/topic/gate-activities/delete', (message) => {
                const id = parseInt(message.body);
                setActivities(prev => prev.filter(a => a.id !== id));
            });
        });

        return () => {
            stompClient.disconnect();
        };
    }, []);

    const formatTime = (timestamp) => {
        const date = new Date(timestamp);
        return date.toLocaleString('de-DE', {
            hour: '2-digit',
            minute: '2-digit',
            day: '2-digit',
            month: '2-digit',
            year: 'numeric',
        });
    };

    const activitiesRecent = activities.slice(Math.max(activities.length - 4, 0)).reverse();

    return (
        <div className="recent-activity">
            <h2 className="recent-title">Letzte Aktivitäten</h2>
            <ul className="activity-list">
                {activitiesRecent.map((activity) => (
                    <li key={activity.id} className="activity-item">
                        <div className="activity-icon">
                            <FiClock size={20} />
                        </div>
                        <div className="activity-content">
                            <span className="activity-message">{activity.message}</span>
                            <span className="activity-time">{formatTime(activity.lastTimeStamp)}</span>
                        </div>
                    </li>
                ))}
            </ul>
        </div>
    );
}

export default RecentActivity;
