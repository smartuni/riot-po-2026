import { InfoBoxes, StatusTables } from '../features/gates';
import { RecentActivity } from '../features/activities';
import { HeaderBar } from '../features/shell';

const DashboardPage = () => {
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

export default DashboardPage
