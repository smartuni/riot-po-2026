import { useState } from 'react';
import { useNavigate, Link } from 'react-router-dom';
import Avatar from '@mui/material/Avatar';
import Button from '@mui/material/Button';
import CssBaseline from '@mui/material/CssBaseline';
import TextField from '@mui/material/TextField';
import Grid from '@mui/material/Grid';
import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';
import Container from '@mui/material/Container';
import { Dialog, DialogTitle, DialogContent, DialogContentText, DialogActions } from '@mui/material';
import api from '../services/api';


const isValidEmail = (email) => /\S+@\S+\.\S+/.test(email);
const isValidPassword = (password) => password.length >= 6;

const LoginPage = () => {
  const navigate = useNavigate();
  const [isErrorDialogOpen, setIsErrorDialogOpen] = useState(false); // state to maintain the error dialog visibility
  const [errorMessage, setErrorMessage] = useState(''); // state to contain the error message
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');


  const handleSubmit = async (event) => {
    event.preventDefault();
    const data = new FormData(event.currentTarget);

    //data format for logging in
    const loginData = {
      email: data.get('email'),
      password: data.get('password'),
    };

    try {
      const response = await api.post('/auth/login', loginData);

      //api call should return a token that is then used in the header
      const { token } = response.data;
      if (response.status === 200) {
        api.interceptors.request.use(
          config => {
            config.headers['Authorization'] = `Bearer ${token}`;
            return config;
          },
          error => {
            return Promise.reject(error);
          }
        )
        localStorage.clear()
        navigate('/dashboard');
      }

    } catch (error) {
      setErrorMessage(error.response.data.error || 'Sorry, an unexpected error occurred');
      setIsErrorDialogOpen(true);
    }
  };

  const isFormValid = isValidEmail(email) && isValidPassword(password);
  return (
    <Container component="main" maxWidth="xs">
      <CssBaseline />
      <Box
        sx={{
          marginTop: 8,
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
        }}
      >
        <Avatar sx={{ m: 1, bgcolor: 'secondary.main' }}>

        </Avatar>
        <Typography component="h1" variant="h5">
          Sign in
        </Typography>
        <Box component="form" onSubmit={handleSubmit} noValidate sx={{ mt: 1 }}>
          <TextField
            margin="normal"
            required
            fullWidth
            id="email"
            label="Email Address"
            name="email"
            autoComplete="email"
            autoFocus
            value={email}
            onChange={(e) => setEmail(e.target.value)}
          />
          <TextField
            margin="normal"
            required
            fullWidth
            name="password"
            label="Password"
            type="password"
            id="password"
            autoComplete="current-password"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
          />
          <Button
            type="submit"
            fullWidth
            variant="contained"
            sx={{ mt: 3, mb: 2 }}
            disabled={!isFormValid}
          >
            Sign In
          </Button>
          <Grid container>
            {/* <Grid item xs>
                                <Link href="/forgot-password" variant="body2">
                                    Forgot password?
                                </Link>
                            </Grid> */
              // future possible addition of forgot password page / functionality
            }
            <Grid>
              <Link to="/register" variant="body2" >
                {"Don't have an account? Sign Up"}
              </Link>
            </Grid>
          </Grid>
        </Box>
      </Box>
      <Dialog
        open={isErrorDialogOpen}
        onClose={() => setIsErrorDialogOpen(false)}
        aria-labelledby="error-dialog-title"
        aria-describedby="error-dialog-description"
        sx={{ '& .MuiDialog-paper': { border: '2px solid red' } }} // This adds the red outline
      >
        <DialogTitle id="error-dialog-title">{"Login Error"}</DialogTitle>
        <DialogContent>
          <DialogContentText id="error-dialog-description">
            {errorMessage}
          </DialogContentText>
        </DialogContent>
        <DialogActions>
          <Button
            onClick={() => setIsErrorDialogOpen(false)}
            color="primary"
            variant="outlined"
          >
            OK
          </Button>
        </DialogActions>
      </Dialog>
    </Container>

  );
}

export default LoginPage;