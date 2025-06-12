import React from 'react';
import { Typography, Paper, List, ListItemButton, ListItemText } from '@mui/material';

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
            <ListItemButton key={index} onClick={() => onNotificationClick(index)}>
              <ListItemText primary={note} />
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
