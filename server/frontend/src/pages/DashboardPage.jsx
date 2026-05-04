import { InfoBoxes } from '../features/gates';
import { StatusTables } from '../features/gates';
import { RecentActivity } from '../features/activities';
import HeaderBar from "../shared/components/HeaderBar";
import { useAuth } from '../features/auth';
import { useEffect, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { AlertDialogIllegal } from '../shared/components/AlertDialogIllegal';

const DashboardPage = () => {
    const [popupOpen, setPopupOpen] = useState(false);
    const navigate = useNavigate();
    const { isAuthenticated, loading } = useAuth();

    const closeDialog = () => {
        navigate('/');
    }

    useEffect(() => {
        if (!loading && !isAuthenticated) {
            setPopupOpen(true);
        }
    }, [loading, isAuthenticated]);

    if (loading) {
        return null; // or a loading spinner
    }
    return (
        <div>
            <HeaderBar />
            <div className="container">
                <div className="main">
                    <InfoBoxes />
                    <div className="content">
                        <StatusTables />
                        <RecentActivity />
                    </div>
                </div>
            </div>
            <AlertDialogIllegal open={popupOpen} onClose={closeDialog}></AlertDialogIllegal>
        </div>
    )
}

export default DashboardPage

