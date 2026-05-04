import { InfoBoxes, StatusTablesView } from '../features/gates';
import { RecentActivity } from '../features/activities';
import HeaderBarGuest from "../shared/components/HeaderBarGuest";


const DashboardGuestPage = () => {

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

export default DashboardGuestPage