import InfoBoxes from './InfoBoxes';
import StatusTables from './StatusTables';
import RecentActivity from "./RecentActivity";
import HeaderBar from "./HeaderBar";
import api from '../services/api';
import { useEffect, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { AlertDialogIllegal } from './AlertDialogIllegal';

function getCookie(name) {
    var nameEQ = name + "=";
    var ca = document.cookie.split(';');
    for(var i=0;i < ca.length;i++) {
        var c = ca[i];
        while (c.charAt(0)==' ') c = c.substring(1,c.length);
        if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
    }
    return null;
}

const Dashboard = () => {
    const [popupOpen, setPopupOpen] = useState(false);
    const navigate = useNavigate();

    var jwt = getCookie("jwt");
    if (jwt != null) {
        api.defaults.headers.common['Authorization'] = `Bearer ${jwt}`;
    }

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
