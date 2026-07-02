import { useState } from 'react';
import { useNavigate, Link } from 'react-router-dom';
import { Dialog, DialogTitle, DialogContent, DialogContentText, DialogActions } from '@mui/material';
import { useRegisterMutation } from '../app/store/api/api';
import { useAppDispatch } from '../app/store';

const RegisterPage = () => {
  const navigate = useNavigate();
  const [register, { isLoading: isRegisterLoading }] = useRegisterMutation();
  const appDispatch = useAppDispatch();

  const [fields, setFields] = useState({
    name: { value: '', touched: false },
    email: { value: '', touched: false },
    password: { value: '', touched: false },
    confirmPassword: { value: '', touched: false }
  });
  const [isErrorDialogOpen, setIsErrorDialogOpen] = useState(false);
  const [errorMessage, setErrorMessage] = useState('');
  const [checked, setChecked] = useState(false);

  const isValidEmail = (email) => /\S+@\S+\.\S+/.test(email);
  const isValidPassword = (password, confirmPassword) => password.length >= 6 && password === confirmPassword;

  const handleFieldChange = (field, value) => {
    setFields(prevFields => ({
      ...prevFields,
      [field]: { value, touched: true }
    }));
  };

  const handleSubmit = async (event) => {
    event.preventDefault();
    const { name, email, password, confirmPassword } = fields;

    if (isValidEmail(email.value) && isValidPassword(password.value, confirmPassword.value)) {
      const role = checked ? 'controller' : 'viewer';
      try {
        const userDetails = await register({
          name: name.value.toString(),
          email: email.value.toString(),
          password: password.value.toString(),
          role,
        }).unwrap();

        appDispatch({ type: 'auth/setUser', payload: userDetails });
        navigate('/dashboard');
      } catch (error) {
        setErrorMessage(error?.data?.error || 'Sorry, an unexpected error occurred');
        setIsErrorDialogOpen(true);
      }
    } else {
      setFields(prevFields => {
        const updated = {};
        Object.keys(prevFields).forEach(k => {
          updated[k] = { ...prevFields[k], touched: true };
        });
        return updated;
      });
    }
  };

  const getFieldError = (field) => {
    if (!fields[field].touched) return '';
    if (field === 'password' && fields[field].value.length < 6) return 'Password must be at least 6 characters';
    if (field === 'confirmPassword' && fields[field].value !== fields.password.value) return 'Passwords do not match!';
    if (field !== 'password' && fields[field].value === '') return `${field.charAt(0).toUpperCase() + field.slice(1)} is required`;
    return '';
  };

  const isFormValid = () => {
    return isValidEmail(fields.email.value) && isValidPassword(fields.password.value, fields.confirmPassword.value);
  };

  return (
    <div className="auth-page">
      <header className="auth-header">
        <Link to="/" style={{ display: 'flex', alignItems: 'center', gap: '10px', color: '#fff', textDecoration: 'none' }}>
          <span className="logo-icon">SM</span>
          <span style={{ fontSize: '18px', fontWeight: 700 }}>SenseMate</span>
        </Link>
      </header>
      <div className="auth-body">
        <div className="auth-card">
          <h1>Create account</h1>
          <p className="subtitle">Get started with SenseMate</p>
          <form onSubmit={handleSubmit}>
            <div className="form-group">
              <label className="form-label" htmlFor="name">Full Name</label>
              <input
                className="form-input"
                type="text"
                id="name"
                placeholder="Max Köhler"
                required
                value={fields.name.value}
                onChange={(e) => handleFieldChange('name', e.target.value)}
              />
              {getFieldError('name') && <p style={{ color: 'var(--red-500)', fontSize: '12px', marginTop: '4px' }}>{getFieldError('name')}</p>}
            </div>
            <div className="form-group">
              <label className="form-label" htmlFor="email">Email</label>
              <input
                className="form-input"
                type="email"
                id="email"
                placeholder="you@hamburg.de"
                required
                value={fields.email.value}
                onChange={(e) => handleFieldChange('email', e.target.value)}
              />
              {getFieldError('email') && <p style={{ color: 'var(--red-500)', fontSize: '12px', marginTop: '4px' }}>{getFieldError('email')}</p>}
            </div>
            <div className="form-group">
              <label className="form-label" htmlFor="password">Password</label>
              <input
                className="form-input"
                type="password"
                id="password"
                placeholder="••••••••"
                required
                value={fields.password.value}
                onChange={(e) => handleFieldChange('password', e.target.value)}
              />
              {getFieldError('password') && <p style={{ color: 'var(--red-500)', fontSize: '12px', marginTop: '4px' }}>{getFieldError('password')}</p>}
            </div>
            <div className="form-group">
              <label className="form-label" htmlFor="confirmPassword">Confirm Password</label>
              <input
                className="form-input"
                type="password"
                id="confirmPassword"
                placeholder="••••••••"
                required
                value={fields.confirmPassword.value}
                onChange={(e) => handleFieldChange('confirmPassword', e.target.value)}
              />
              {getFieldError('confirmPassword') && <p style={{ color: 'var(--red-500)', fontSize: '12px', marginTop: '4px' }}>{getFieldError('confirmPassword')}</p>}
            </div>
            <div className="form-group" style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
              <label className="toggle-switch">
                <input type="checkbox" checked={checked} onChange={() => setChecked(!checked)} />
                <span className="toggle-slider"></span>
              </label>
              <span className="form-label" style={{ margin: 0 }}>Request controller access</span>
            </div>
            <button className="btn btn-primary" type="submit" disabled={!isFormValid() || isRegisterLoading}>
              Create Account
            </button>
          </form>
          <div className="auth-footer">
            Already have an account? <Link to="/login">Sign In</Link>
          </div>
        </div>
      </div>
      <Dialog open={isErrorDialogOpen} onClose={() => setIsErrorDialogOpen(false)}>
        <DialogTitle>{"Registration Error"}</DialogTitle>
        <DialogContent>
          <DialogContentText>{errorMessage}</DialogContentText>
        </DialogContent>
        <DialogActions>
          <button className="btn btn-primary" onClick={() => setIsErrorDialogOpen(false)} style={{ marginTop: 0 }}>OK</button>
        </DialogActions>
      </Dialog>
    </div>
  );
};

export default RegisterPage;
