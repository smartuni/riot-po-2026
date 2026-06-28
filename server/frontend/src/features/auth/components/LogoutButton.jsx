import { useState } from 'react';
import { Alert, Button } from "@mui/material";
import { useLogoutMutation } from '../../../app/store/api/api';
import { useNavigate } from "react-router-dom";
import { FiLogOut } from "react-icons/fi";

const LogoutButton = () => {
  const navigate = useNavigate();
  const [logout] = useLogoutMutation();
  const [errorMsg, setErrorMsg] = useState(null);
  const handleLogout = async () => {
    try {
      await logout().unwrap();
      navigate(`/`);
    }
    catch (error) {
      setErrorMsg('Logout failed. Please try again.');
      console.error('Logout failed:', error);
    }
  }

  return (
    <>
      {errorMsg && <Alert severity="error" onClose={() => setErrorMsg(null)} sx={{ mb: 1 }}>{errorMsg}</Alert>}
      <Button onClick={handleLogout} startIcon={<FiLogOut />} sx={{
        borderRadius: 2, width: '120px', height: '60px', textTransform: 'none', fontSize: '18px', '&:hover': { backgroundColor: 'var(--accent-hover)' }
      }}>
        Logout
      </Button>
    </>
  )
}

export default LogoutButton;
