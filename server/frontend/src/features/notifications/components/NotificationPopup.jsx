import React from 'react';
import { Typography, Paper, List, ListItemButton, ListItemText } from '@mui/material';
import CheckIcon from '@mui/icons-material/Check'; // Optional: Icon für "gelesen"

const NotificationPopup = ({ notifications, onNotificationClick }) => {
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
                backgroundColor: 'background.paper',
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
                            onClick={() => onNotificationClick(index)}
                            sx={{
                                opacity: note.read ? 0.6 : 1, // Gelesene sind "blasser"
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
