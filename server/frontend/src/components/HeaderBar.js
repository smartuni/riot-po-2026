import React, { useState, useEffect, useRef } from "react";
import '../styles/HeaderBar.css';
import {FiHome, FiLock, FiMap, FiClipboard, FiActivity, FiUser, FiBell, FiGrid} from 'react-icons/fi';
import { Button, Badge } from "@mui/material";
import { useNavigate } from 'react-router-dom';
import NotificationPopup from './NotificationPopup';
import {fetchGates, fetchNotification} from "../services/api";

function HeaderBar() {
    const navigate = useNavigate();
    const [notifications, setNotifications] = useState([]);
    const [popupVisible, setPopupVisible] = useState(false);
    const popupRef = useRef();

    useEffect(() => {
        const loadNotifications = async () => {
            try {
                const data = await fetchNotification();
                setNotifications(data);
            } catch (error) {
                console.error('Fehler beim Laden der Nachrichten', error);
            }
        };
        loadNotifications();
    }, []);

    const togglePopup = () => {
        setPopupVisible(prev => !prev);
    };

    const handleNotificationClick = (index) => {
        const clickedMessage = notifications[index];
        alert(`Opening notification: ${clickedMessage}`);
        // You can navigate somewhere or update the state to show details
    };

    const handleClickOutside = (event) => {
        if (popupRef.current && !popupRef.current.contains(event.target)) {
            setPopupVisible(false);
        }
    };

    useEffect(() => {
        if (popupVisible) {
            document.addEventListener("mousedown", handleClickOutside);
        } else {
            document.removeEventListener("mousedown", handleClickOutside);
        }
        return () => {
            document.removeEventListener("mousedown", handleClickOutside);
        };
    }, [popupVisible]);

    return (
        <div style={{ position: 'relative' }}>
            <div className="header-bar">
                <div className="logo">
                    <Button startIcon={<FiHome />} size="large" color="white" onClick={() => navigate('/dashboard')}></Button>
                    <div className="logo-title">
                        <h1>
                            SenseMate
                        </h1>
                        <p>
                            Gate Management Dashboard
                        </p>
                    </div>
                </div>
                <div className="profile-notify-icons">
                    <Badge
                        badgeContent={notifications.length}
                        color="error"
                        overlap="circular"
                    >
                        <Button
                            id='notificationButton'
                            onClick={togglePopup}
                            sx={{ minWidth: '40px', padding: 1, color: 'white', '&:hover': { backgroundColor: '#115293' } }}
                        >
                            <FiBell size={20} />
                        </Button>
                    </Badge>

                    <Button
                        id='userButton'
                        onClick={() => navigate('/userpage')}
                        sx={{ minWidth: '70px', padding: 1, color: 'white', '&:hover': { backgroundColor: '#115293' } }}
                    >
                        <FiUser size={20} />
                    </Button>
                </div>
            </div>

            {popupVisible && (
                <div ref={popupRef}>
                    <NotificationPopup
                        notifications={notifications}
                        onNotificationClick={handleNotificationClick}
                    />
                </div>
            )}
        </div>
    );
}

export default HeaderBar;
