import { Navigate, Outlet } from 'react-router-dom';
import { Box, CircularProgress } from '@mui/material';
import { useAppSelector } from '../../../app/store';

const PublicOnlyRoute = ({ children }) => {
  const { status, user } = useAppSelector((state) => state.auth);

  if (status === 'loading') {
    return (
      <Box sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', minHeight: '100vh' }}>
        <CircularProgress />
      </Box>
    );
  }

  if (status === 'authenticated') {
    const target = '/dashboard';
    return <Navigate to={target} replace />;
  }

  return children || <Outlet />;
};

export default PublicOnlyRoute;
