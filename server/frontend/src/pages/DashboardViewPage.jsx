import { InfoBoxes } from '../features/gates';
import { RecentActivity } from '../features/activities';
import { HeaderBar } from '../features/shell';
import { StatusTablesView } from '../features/gates';
import { apiClient, getCookie } from '../shared';
import { useEffect, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { AlertDialogIllegal } from '../shared';

const DashboardViewPage = () => {
    const [popupOpen, setPopupOpen] = useState(false);
    const navigate = useNavigate();

    var jwt = getCookie("jwt");
    if (jwt != null) {
        apiClient.defaults.headers.common['Authorization'] = `Bearer ${jwt}`;
    }

    const loadDetails = async () => {
        try {
            const response = await apiClient.get('/auth/user-details');
            if (response.status !== 200) {
                throw new Error('Request failed with status code ' + response.status);
            }
        } catch (e) {
            setPopupOpen(true);
        }
    };

    useEffect(() => {
        loadDetails();
    }, []);

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
