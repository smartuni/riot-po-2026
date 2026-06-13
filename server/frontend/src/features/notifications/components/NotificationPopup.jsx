import React from 'react';
import { Typography, Paper, List, ListItemButton, ListItemText, CircularProgress, Alert } from '@mui/material';
import CheckIcon from '@mui/icons-material/Check';
import { useAppSelector } from '../../../app/store';
import {
    useGetNotificationsByWorkerIdQuery,
    useMarkNotificationAsReadMutation,
} from '../../../app/store/api/api';

const NotificationPopup = () => {
    const userDetails = useAppSelector((state) => state.auth.user);
    const workerId = userDetails?.workerId ?? null;
    const { data: notificationsData, isLoading: notificationsLoading, error: notificationsError } = useGetNotificationsByWorkerIdQuery(workerId, {
        skip: !workerId,
    });
    const [markAsRead] = useMarkNotificationAsReadMutation();

    if (notificationsLoading) {
        return (
            <Paper
                elevation={6}
                sx={{
                    position: 'absolute',
                    top: 60,
                    right: 20,
                    width: 300,
                    zIndex: 2000,
                    borderRadius: 2,
                    p: 2,
                }}
            >
                <div style={{ display: 'flex', justifyContent: 'center', padding: '1rem' }}>
                    <CircularProgress size={24} />
                </div>
            </Paper>
        );
    }

    if (notificationsError) {
        return (
            <Paper
                elevation={6}
                sx={{
                    position: 'absolute',
                    top: 60,
                    right: 20,
                    width: 300,
                    zIndex: 2000,
                    borderRadius: 2,
                    p: 2,
                }}
            >
                <Alert severity="error">Failed to load notifications</Alert>
            </Paper>
        );
    }

    const notifications = notificationsData ?? [];

    const handleNotificationClick = async (notification) => {
        if (!notification.read) {
            try {
                await markAsRead(notification.id).unwrap();
            } catch (error) {
                console.error("Fehler beim Aktualisieren der Benachrichtigung:", error);
            }
        }
    };

    return (
        <Paper
            elevation={6}
            sx={{
                position: 'absolute',
                top: 60,
                right: 20,
                width: 300,
                zIndex: 2000,
                borderRadius: 2,
                p: 2,
            }}
        >
            <Typography variant="h6" sx={{ mb: 1 }}>
                Notifications
            </Typography>
            <List dense>
                {notifications.length > 0 ? (
                    notifications.map((note, index) => (
                        <ListItemButton
                            key={index}
                            onClick={() => handleNotificationClick(note)}
                            sx={{
                                opacity: note.read ? 0.6 : 1,
                                alignItems: 'flex-start'
                            }}
                        >
                            <ListItemText
                                primary={
                                    <Typography
                                        sx={{
                                            fontWeight: note.read ? 'normal' : 'bold',
                                            display: 'flex',
                                            alignItems: 'center'
                                        }}
                                    >
                                        {note.message}
                                        {note.read && <CheckIcon sx={{ fontSize: 16, ml: 1 }} />}
                                    </Typography>
                                }
                                secondary={new Date(note.lastTimeStamp).toLocaleString()}
                            />
                        </ListItemButton>
                    ))
                ) : (
                    <Typography variant="body2" sx={{ p: 1, color: 'text.secondary' }}>
                        No new notifications
                    </Typography>
                )}
            </List>
        </Paper>
    );
};

export default NotificationPopup;
