import { Navigate, Outlet } from 'react-router-dom';
import { useAppSelector } from '../../../app/store';

const PublicOnlyRoute = ({ children }) => {
  const { status, user } = useAppSelector((state) => state.auth);

  if (status === 'authenticated') {
    const target = user?.role === 'controller' ? '/dashboard' : '/dashboard-view';
    return <Navigate to={target} replace />;
  }

  return children || <Outlet />;
};

export default PublicOnlyRoute;
