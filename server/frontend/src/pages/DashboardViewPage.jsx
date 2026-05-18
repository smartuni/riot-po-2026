import { InfoBoxes, StatusTablesView } from '../features/gates';
import { RecentActivity } from '../features/activities';
import { HeaderBar } from '../features/shell';
import { useNavigate } from 'react-router-dom';
import { AlertDialogIllegal } from '../shared';
import { useGetUserDetailsQuery } from '../app/store/api/api';

const DashboardViewPage = () => {
    const navigate = useNavigate();

    const { error } = useGetUserDetailsQuery();
    const popupOpen = !!error;

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
