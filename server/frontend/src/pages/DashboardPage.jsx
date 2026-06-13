import AppLayout from '../features/shell/components/AppLayout';
import StatCards from '../features/gates/components/StatCards';
import StatusTables from '../features/gates/components/StatusTables';
import StatusTablesView from '../features/gates/components/StatusTablesView';
import ActivityPanel from '../features/activities/components/ActivityPanel';
import { useAppSelector } from '../app/store';

const DashboardPage = () => {
  const user = useAppSelector((state) => state.auth.user);
  const isController = user?.role === 'controller';

  return (
    <AppLayout>
      <StatCards />
      <div className="dashboard-grid">
        {isController ? <StatusTables /> : <StatusTablesView />}
        <ActivityPanel />
      </div>
    </AppLayout>
  );
};

export default DashboardPage;
