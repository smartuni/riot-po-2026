import '../styles/HeaderBar.css';
import { FiXCircle } from 'react-icons/fi';
import { Button } from "@mui/material";
import { useNavigate } from 'react-router-dom';

function HeaderBarGuest() {
    const navigate = useNavigate();

    return (
        <div style={{ position: 'relative' }}>
            <div className="header-bar">
                <div className="logo">
                    <div className="logo-title">
                        <h1>SenseMate</h1>
                        <p>Gate Management Dashboard</p>
                    </div>
                </div>

                <div className="exit">
                    <Button
                        id='exitButton'
                        onClick={() => navigate('/')}
                        sx={{ minWidth: '70px', minHeight: '70px', padding: 1, color: 'white', '&:hover': { backgroundColor: '#115293' } }}
                    >
                        <FiXCircle size={30} />
                    </Button>
                </div>
            </div>
        </div>
    );
}

export default HeaderBarGuest;
