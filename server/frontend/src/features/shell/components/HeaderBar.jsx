import React, { useState, useEffect, useRef } from "react";
import '../../../shared/styles/HeaderBar.css';
import { FiHome, FiUser, FiBell } from 'react-icons/fi';
import {
    Button, Badge, Dialog, DialogTitle,
    DialogContent, DialogContentText, DialogActions
} from "@mui/material";
import { useNavigate } from 'react-router-dom';
import { NotificationPopup } from '../../notifications';
import {
    useGetUserDetailsQuery,
    useGetNotificationsByWorkerIdQuery,
    useMarkNotificationAsReadMutation,
} from '../../../app/store/api/api';
import { CircularProgress } from '@mui/material';
import { useAppSelector } from '../../../app/store';

function HeaderBar() {
    const navigate = useNavigate();
    const [popupVisible, setPopupVisible] = useState(false);
    const popupRef = useRef();

    const [dialogOpen, setDialogOpen] = useState(false);
    const [selectedNotification, setSelectedNotification] = useState(null);

    const { data: userDetails, isLoading: userLoading, error: userError } = useGetUserDetailsQuery();
    const workerId = userDetails?.workerId ?? null;

    const { data: notificationsData, isLoading: notificationsLoading, error: notificationsError } = useGetNotificationsByWorkerIdQuery(workerId, {
        skip: !workerId,
    });

    const [markAsRead] = useMarkNotificationAsReadMutation();

    if (userLoading || notificationsLoading) {
        return (
            <div className="header-bar" style={{ display: 'flex', alignItems: 'center', justifyContent: 'center', minHeight: '60px' }}>
                <CircularProgress size={24} sx={{ color: 'white' }} />
            </div>
        );
    }

    if (userError || notificationsError) {
        return (
            <div className="header-bar" style={{ display: 'flex', alignItems: 'center', justifyContent: 'center', minHeight: '60px', backgroundColor: '#f44336' }}>
                <span style={{ color: 'white' }}>Error loading user data</span>
            </div>
        );
    }

    const notifications = notificationsData ?? [];
    const numberOfUnreadNotifications = notifications.filter(n => !n.read).length;

    const togglePopup = () => {
        setPopupVisible(prev => !prev);
    };

    const handleNotificationClick = async (index) => {
        const clicked = notifications[index];

        if (!clicked.read) {
            try {
                await markAsRead(clicked.id).unwrap();
            } catch (error) {
                console.error("Fehler beim Aktualisieren der Benachrichtigung:", error);
                return;
            }
        }

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
                    <NotificationPopup />
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

