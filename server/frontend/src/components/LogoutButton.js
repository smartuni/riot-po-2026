import { Button } from "@mui/material";
import api from '../services/api';
import { useNavigate } from "react-router-dom";
import { FiLogOut } from "react-icons/fi";


const LogoutButton = () => {
  const navigate = useNavigate();
  const handleLogout = async () => {
    try {
      const response = await api.post('/auth/logout');
      if (response.status !== 200) {
        throw new Error('Request failed with status code ' + response.status);
      }
      if (response.status === 200) {
        api.interceptors.request.use(
          config => {
            config.headers['Authorization'] = ``;
            return config;
          },
          error => {
            return Promise.reject(error);
          }
        )
      }

    }
    catch (e) {
      console.log(e)
    }
    navigate(`/`)

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