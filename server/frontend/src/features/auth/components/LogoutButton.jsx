import { Button } from "@mui/material";
import { useLogoutMutation } from '../../../app/store/api/api';
import { useNavigate } from "react-router-dom";
import { FiLogOut } from "react-icons/fi";

const LogoutButton = () => {
  const navigate = useNavigate();
  const [logout] = useLogoutMutation();
  const handleLogout = async () => {
    try {
      await logout().unwrap();
    }
    catch (e) {
      console.log(e);
    }
    navigate(`/`);
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
