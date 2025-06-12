import Sidebar from './Sidebar';
import InfoBoxes from './InfoBoxes';
import StatusTables from './StatusTables';
import MapView from './MapView';
import RecentActivity from "./RecentActivity";
import HeaderBar from "./HeaderBar";


const Dashboard = () => {

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
        </div>
    )
}

export default Dashboard