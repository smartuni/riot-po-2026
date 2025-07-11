import React, { useState, useEffect, useRef } from "react";
import '../styles/HeaderBar.css';
import { FiHome, FiUser, FiBell } from 'react-icons/fi';
import {
    Button, Badge, Dialog, DialogTitle,
    DialogContent, DialogContentText, DialogActions
} from "@mui/material";
import { useNavigate } from 'react-router-dom';
import NotificationPopup from './NotificationPopup';
import api, { fetchNotificationByWorkerId, markNotificationAsRead } from "../services/api";
import SockJS from 'sockjs-client';
import { Stomp } from '@stomp/stompjs';

function HeaderBar() {
    const navigate = useNavigate();
    const [notifications, setNotifications] = useState([]);
    const [popupVisible, setPopupVisible] = useState(false);
    const [workerId, setWorkerId] = useState(null);
    const popupRef = useRef();

    const [dialogOpen, setDialogOpen] = useState(false);
    const [selectedNotification, setSelectedNotification] = useState(null);

    const numberOfUnreadNotifications = notifications.filter(n => !n.read).length;



    // useEffect(() => {
    //     const socket = new SockJS('http://localhost:8080/ws');
    //     const stompClient = Stomp.over(socket);

    //     stompClient.connect({}, () => {
    //         stompClient.subscribe('/topic/notifications', (message) => {
    //             const newNotification = JSON.parse(message.body);
    //             setNotifications(prev => [...prev, newNotification]);
    //         });

    //         stompClient.subscribe('/topic/notifications/delete', (message) => {
    //             const deletedId = message.body;
    //             setNotifications(prev => prev.filter(n => n.id !== parseInt(deletedId)));
    //         });

    //         stompClient.subscribe('/topic/notifications/read-change', (message) => {
    //             const updated = JSON.parse(message.body);
    //             setNotifications(prev =>
    //                 prev.map(n => n.id === updated.id ? updated : n)
    //             );
    //         });
    //     });

    //     return () => stompClient.disconnect();
    // }, []);


    useEffect(() => {
        const loadDetails = async () => {
            try {
                const response = await api.get('/auth/user-details');
                if (response.status !== 200) {
                    throw new Error('Request failed with status code ' + response.status);
                }
                setWorkerId(response.data.workerId);
            } catch (e) {
                console.error("Fehler beim Laden der User-Details:", e);
            }
        };

        loadDetails();
    }, []);

    useEffect(() => {
        if (workerId !== null) {
            const loadNotifications = async () => {
                try {
                    const data = await fetchNotificationByWorkerId(workerId);
                    setNotifications(data);
                } catch (error) {
                    console.error('Fehler beim Laden der Nachrichten:', error);
                }
            };
            loadNotifications();
        }
    }, [workerId]);

    const togglePopup = () => {
        setPopupVisible(prev => !prev);
    };

    const handleNotificationClick = async (index) => {
        const clicked = notifications[index];

        // Prüfen, ob die Nachricht bereits gelesen wurde
        if (!clicked.read) {
            try {
                await markNotificationAsRead(clicked.id);

                // UI-Update lokal
                const updatedNotifications = [...notifications];
                updatedNotifications[index] = { ...clicked, read: true };
                setNotifications(updatedNotifications);
            } catch (error) {
                console.error("Fehler beim Aktualisieren der Benachrichtigung:", error);
                return;
            }
        }

        // In jedem Fall: Dialog öffnen
        setSelectedNotification(clicked);
        setDialogOpen(true);
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
                    <Button
                        startIcon={<FiHome />}
                        size="large"
                        color="white"
                        onClick={() => navigate('/dashboard')}
                    />
                    <div className="logo-title">
                        <h1>SenseMate</h1>
                        <p>Gate Management Dashboard</p>
                    </div>
                </div>

                <div className="profile-notify-icons">
                    <Badge
                        badgeContent={numberOfUnreadNotifications}
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

            <Dialog open={dialogOpen} onClose={() => setDialogOpen(false)}>
                <DialogTitle>Benachrichtigung</DialogTitle>
                <DialogContent>
                    <DialogContentText sx={{ fontSize: '1rem', color: 'black' }}>
                        {selectedNotification?.message}
                    </DialogContentText>
                    <DialogContentText sx={{ fontSize: '0.8rem', mt: 2, color: 'grey' }}>
                        {selectedNotification && new Date(selectedNotification.lastTimeStamp).toLocaleString()}
                    </DialogContentText>
                </DialogContent>
                <DialogActions>
                    <Button onClick={() => setDialogOpen(false)} variant="contained">
                        Schließen
                    </Button>
                </DialogActions>
            </Dialog>
        </div>
    );
}

export default HeaderBar;
