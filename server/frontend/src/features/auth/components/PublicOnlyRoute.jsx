import { Navigate, Outlet } from 'react-router-dom';
import { useAppSelector } from '../../../app/store';

const PublicOnlyRoute = ({ children }) => {
  const { status } = useAppSelector((state) => state.auth);

  if (status === 'authenticated') {
    return <Navigate to="/dashboard" replace />;
  }

  return children || <Outlet />;
};

export default PublicOnlyRoute;
