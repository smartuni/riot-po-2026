import InfoBoxes from './InfoBoxes';
import RecentActivity from "./RecentActivity";
import HeaderBar from "./HeaderBar";
import StatusTablesView from './StatusTablesView';


const DashboardView = () => {

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
        </div>
    )
}

export default DashboardView