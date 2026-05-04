import { Button } from "@mui/material";
import { useNavigate } from "react-router-dom";
import { FiLogOut } from "react-icons/fi";
import { useAuth } from "../useAuth";

const LogoutButton = () => {
  const navigate = useNavigate();
  const { logout } = useAuth();

  const handleLogout = async () => {
    await logout();
    navigate('/');
  }
  return (
    <Button onClick={handleLogout} startIcon={<FiLogOut />} sx={{
      borderRadius: 2, width: '120px', height: '60px', color: 'white', textTransform: 'none', fontSize: '18px', '&:hover': { backgroundColor: '#115293' }
    }}>
      Logout
    </Button>
  )
}

export default LogoutButton;

