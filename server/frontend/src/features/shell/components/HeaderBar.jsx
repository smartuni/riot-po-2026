import React, { useState, useEffect, useRef } from "react";
import '../../../shared/styles/HeaderBar.css';
import { FiHome, FiUser, FiBell } from 'react-icons/fi';
import {
    Button, Badge, CircularProgress
} from "@mui/material";
import { useNavigate } from 'react-router-dom';
import { NotificationPopup } from '../../notifications';
import {
    useGetUserDetailsQuery,
    useGetNotificationsByWorkerIdQuery,
} from '../../../app/store/api/api';

function HeaderBar() {
    const navigate = useNavigate();
    const [popupVisible, setPopupVisible] = useState(false);
    const popupRef = useRef();

    const { data: userDetails, isLoading: userLoading, error: userError } = useGetUserDetailsQuery();
    const workerId = userDetails?.workerId ?? null;

    const { data: notificationsData, isLoading: notificationsLoading, error: notificationsError } = useGetNotificationsByWorkerIdQuery(workerId, {
        skip: !workerId,
    });

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
        </div>
    );
}

export default HeaderBar;
