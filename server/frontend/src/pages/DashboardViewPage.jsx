import { InfoBoxes } from '../features/gates';
import { RecentActivity } from '../features/activities';
import { HeaderBar } from '../features/shell';
import { StatusTablesView } from '../features/gates';
import { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { AlertDialogIllegal } from '../shared';
import { useGetUserDetailsQuery } from '../app/store/api/api';

const DashboardViewPage = () => {
    const [popupOpen, setPopupOpen] = useState(false);
    const navigate = useNavigate();

    const { error } = useGetUserDetailsQuery();

    useEffect(() => {
        if (error) {
            setPopupOpen(true);
        }
    }, [error]);

    const closeDialog = () => {
        navigate('/');
    }

    return (
        <div>
            <HeaderBar />
            <div className="container">
                <div className="main">
                    <InfoBoxes />
                    <div className="content">
                        <StatusTablesView />
                        <RecentActivity />
                    </div>
                </div>
            </div>
            <AlertDialogIllegal open={popupOpen} onClose={closeDialog}></AlertDialogIllegal>
        </div>
    )
}

export default DashboardViewPage
