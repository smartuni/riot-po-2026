import InfoBoxes from './InfoBoxes';
import StatusTables from './StatusTables';
import RecentActivity from "./RecentActivity";
import HeaderBar from "./HeaderBar";
import api from '../services/api';
import { useEffect, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { AlertDialogIllegal } from './AlertDialogIllegal';


const Dashboard = () => {
    const [popupOpen, setPopupOpen] = useState(false);
    const navigate = useNavigate();


    const loadDetails = async () => {
        try {
            const response = await api.get('/auth/user-details');
            if (response.status !== 200) {
                throw new Error('Request failed with status code ' + response.status);
            }
        }
        catch (e) {
            setPopupOpen(true);
            console.log('popup');
            console.log(e)
        }
    }
    const closeDialog = () => {
        navigate('/');
    }

    useEffect(() => {
        //api call here to get user profile on useState (when first mounted)
        loadDetails();
    }, []);

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

export default Dashboard