import InfoBoxes from './InfoBoxes';
import RecentActivity from "./RecentActivity";
import HeaderBarGuest from "./HeaderBarGuest";
import StatusTablesView from './StatusTablesView';


const DashboardGuest = () => {

    return (
        <div>
            <HeaderBarGuest />
            <div className="container">
                <div className="main">
                    <InfoBoxes />
                    <div className="content">
                        <StatusTablesView />
                        <RecentActivity />
                    </div>
                </div>
            </div>
        </div>
    )
}

export default DashboardGuest