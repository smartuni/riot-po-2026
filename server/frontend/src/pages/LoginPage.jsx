import { useState } from 'react';
import { useNavigate, Link } from 'react-router-dom';
import { Dialog, DialogTitle, DialogContent, DialogContentText, DialogActions } from '@mui/material';
import { useLoginMutation } from '../app/store/api/api';
import { useAppDispatch } from '../app/store';

const isValidEmail = (email) => /\S+@\S+\.\S+/.test(email);
const isValidPassword = (password) => password.length >= 6;

const LoginPage = () => {
  const navigate = useNavigate();
  const [isErrorDialogOpen, setIsErrorDialogOpen] = useState(false);
  const [errorMessage, setErrorMessage] = useState('');
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [login, { isLoading: isLoginLoading }] = useLoginMutation();
  const appDispatch = useAppDispatch();

  const handleSubmit = async (event) => {
    event.preventDefault();
    const data = new FormData(event.currentTarget);

    try {
      const userDetails = await login({
        email: data.get('email'),
        password: data.get('password'),
      }).unwrap();

      appDispatch({ type: 'auth/setUser', payload: userDetails });
      navigate('/dashboard');
    } catch (error) {
      setErrorMessage(error?.data?.error || 'Sorry, an unexpected error occurred');
      setIsErrorDialogOpen(true);
    }
  };

  const isFormValid = isValidEmail(email) && isValidPassword(password);

  return (
    <div className="auth-page">
      <header className="auth-header">
        <Link to="/">
          <div className="logo-icon">SM</div>
          <span>SenseMate</span>
        </Link>
      </header>
      <div className="auth-body">
        <div className="auth-card">
          <h1>Welcome back</h1>
          <p className="subtitle">Sign in to your SenseMate account</p>
          <form onSubmit={handleSubmit}>
            <div className="form-group">
              <label className="form-label" htmlFor="email">Email</label>
              <input
                className="form-input"
                type="email"
                id="email"
                name="email"
                placeholder="you@hamburg.de"
                required
                value={email}
                onChange={(e) => setEmail(e.target.value)}
              />
            </div>
            <div className="form-group">
              <label className="form-label" htmlFor="password">Password</label>
              <input
                className="form-input"
                type="password"
                id="password"
                name="password"
                placeholder="••••••••"
                required
                value={password}
                onChange={(e) => setPassword(e.target.value)}
              />
            </div>
            <button
              className="btn btn-primary"
              type="submit"
              disabled={!isFormValid || isLoginLoading}
            >
              Sign In
            </button>
          </form>
          <div className="auth-footer">
            Don't have an account? <Link to="/register">Register</Link>
          </div>
        </div>
      </div>
      <Dialog
        open={isErrorDialogOpen}
        onClose={() => setIsErrorDialogOpen(false)}
        aria-labelledby="error-dialog-title"
        aria-describedby="error-dialog-description"
        sx={{ '& .MuiDialog-paper': { border: '2px solid red' } }}
      >
        <DialogTitle id="error-dialog-title">Login Error</DialogTitle>
        <DialogContent>
          <DialogContentText id="error-dialog-description">
            {errorMessage}
          </DialogContentText>
        </DialogContent>
        <DialogActions>
          <button
            onClick={() => setIsErrorDialogOpen(false)}
            className="btn btn-outline"
          >
            OK
          </button>
        </DialogActions>
      </Dialog>
    </div>
  );
};

export default LoginPage;
