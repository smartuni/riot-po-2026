import { useState } from 'react';
import { useNavigate, Link } from 'react-router-dom';
import { Avatar, Button, CssBaseline, TextField, Box, Typography, Container, Dialog, DialogTitle, DialogContent, DialogContentText, DialogActions, Switch, FormControlLabel } from '@mui/material';
import Grid from '@mui/material/Grid';
import api from '../services/api';

const RegisterPage = () => {
  const navigate = useNavigate();
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
      var role = 'viewer';
      if (checked) {
        role = 'controller';
      }
      try {
        const registerData = {
          email: email.value.toString(),
          password: password.value.toString(),
          name: name.value.toString(),
          role: role,
        };
        const response = await api.post('/auth/register', registerData);
        // Assume a successful registration redirects to the login page
        const { token } = response.data;
        // console.log(token);
        if (response.status === 200) {
          api.defaults.headers.common['Authorization'] = `Bearer ${token}`;

          if (role === 'controller') {
            navigate('/dashboard');
          } else {
            navigate('/dashboard-view');
          }
        }
      } catch (error) {
        setErrorMessage(error.response.data.error || 'Sorry, an unexpected error occurred');
        console.log(error);
        setIsErrorDialogOpen(true);
      }
    } else {
      // Trigger visual feedback for invalid fields
      Object.keys(fields).forEach(key => {
        fields[key].touched = true;
      });
      setFields({ ...fields });
    }
  };

  const formField = (field, label, type = 'text', autoComplete = 'off') => (
    <TextField
      variant="outlined"
      margin="normal"
      required
      fullWidth
      id={field}
      label={label}
      name={field}
      autoComplete={autoComplete}
      type={type}
      value={fields[field].value}
      onChange={(e) => handleFieldChange(field, e.target.value)}
      error={(field === 'password' && fields[field].value.length < 6 && fields[field].touched) ||
        (field !== 'password' && fields[field].value === '' && fields[field].touched)}
      helperText={(field === 'password' && fields[field].value.length < 6 && fields[field].touched) ? 'Password must be at least 6 characters' : (field === 'confirmPassword' && fields[field].value !== fields.password.value && fields[field].touched) ? 'Passwords do not match!' : ''}
      sx={{
        '& .MuiOutlinedInput-root': {
          '& fieldset': {
            borderColor: ((field === 'password' && fields[field].value.length < 6 && fields[field].touched) ||
              (field !== 'password' && fields[field].value === '' && fields[field].touched) ||
              (field === 'confirmPassword' && fields[field].value !== fields.password.value && fields[field].touched)) ? 'red' : ''
          },
        },

      }}
    />
  );

  const handleChangeCheck = () => {
    setChecked(!checked);
  }

  const isFormValid = () => {
    return isValidEmail(fields.email.value) && isValidPassword(fields.password.value, fields.confirmPassword.value);
  };

  return (

    <Container component="main" maxWidth="xs">
      <CssBaseline />
      <Box sx={{ marginTop: 8, display: 'flex', flexDirection: 'column', alignItems: 'center', }}>
        <Avatar sx={{ m: 1, bgcolor: 'secondary.main' }}></Avatar>
        <Typography component="h1" variant="h5">Sign Up</Typography>
        <Box component="form" onSubmit={handleSubmit} sx={{ width: 1 }}>
          <Grid container spacing={2} direction={'column'}>
            <Grid>
              {formField('name', 'Name')}
            </Grid>
            <Grid>
              {formField('email', 'Email Address', 'email', 'off')}
            </Grid>
            <Grid>
              {formField('password', 'Password', 'password')}
            </Grid>
            <Grid>
              {formField('confirmPassword', 'Confirm Password', 'password')}
            </Grid>
            <Grid>
              <FormControlLabel control={<Switch checked={checked} onChange={handleChangeCheck}/>} label={'Controller?'}></FormControlLabel>
            </Grid>
          </Grid>
          <Button
            type="submit"
            fullWidth
            variant="contained"
            sx={{ mt: 3, mb: 2 }}
            disabled={!isFormValid()}
          >
            Sign Up
          </Button>
          <Grid container justifyContent="flex-end">
            <Grid>
              <Link to="/login" variant="body2">Already have an account? Sign in</Link>
            </Grid>
          </Grid>
        </Box>
      </Box>
      <Dialog open={isErrorDialogOpen} onClose={() => setIsErrorDialogOpen(false)}>
        <DialogTitle>{"Registration Error"}</DialogTitle>
        <DialogContent>
          <DialogContentText>{errorMessage}</DialogContentText>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setIsErrorDialogOpen(false)}>OK</Button>
        </DialogActions>
      </Dialog>
    </Container>

  );
}

export default RegisterPage;
