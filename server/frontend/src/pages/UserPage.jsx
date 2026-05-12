import { Button, Grid, Box, TextField } from "@mui/material";
import { HeaderBar } from '../features/shell';
import { LogoutButton } from "../features/auth";
import { useGetUserDetailsQuery, useUpdateUserDetailsMutation } from '../app/store/api/api';
import { useNavigate } from "react-router-dom";
import { FiRotateCcw, FiUser } from "react-icons/fi";
import { useState, useEffect } from "react";
import { AlertDialog } from "../shared";

const UserPage = () => {
    const navigate = useNavigate();
    const { data: userDetails } = useGetUserDetailsQuery();
    const [updateUserDetails, { isLoading: isUpdateLoading }] = useUpdateUserDetailsMutation();
    const [username, setUsername] = useState('');
    const [displayName, setDisplayName] = useState('');
    const [values, setValues] = useState({ name: '', password: '', newPassword: '' });
    const [role, setRole] = useState('');
    const [confirmationOpen, setConfirmationOpen] = useState(false);

    const isFormValid = () => {
        return values.password.length > 6 && values.name.length > 0;
    };

    useEffect(() => {
        if (userDetails) {
            setUsername(userDetails.email);
            setDisplayName(userDetails.name);
            setRole(userDetails.role);
            setValues({ name: userDetails.name, password: '', newPassword: '' });
        }
    }, [userDetails]);

    const handleSubmit = async (event) => {
        event.preventDefault();
        try {
            await updateUserDetails({
                name: values.name,
                password: values.password,
                newPassword: values.newPassword,
            }).unwrap();
            setConfirmationOpen(true);
        }
        catch (e) {
            console.log(e);
        }
    }

    const closeDialog = () => {
        setConfirmationOpen(false)
    }

    const handleChange = (value) => (event) => {
        setValues({ ...values, [value]: event.target.value });
    };

    const handleBack = () => {
        if (role === 'controller') {
            navigate('/dashboard');
        } else {
            navigate('/dashboard-view');
        }
    }

    const error = values.password.length < 6 && values.password !== '';

    return (
        <div>
            <HeaderBar />
            <div className="container" style={{ alignItems: 'flex-start', justifyContent: 'center', marginTop: '40px' }}>
                <div id="userinformation" style={{ display: 'flex', justifyContent: 'center', alignItems: 'center', flexDirection: 'column' }}>
                    <FiUser style={{ width: '100px', height: '100px' }}></FiUser>
                    <div style={{ display: 'flex', alignItems: 'center', gap: '20px' }}>
                        <h2 >Username:</h2>
                        <p >{username}</p>
                    </div>
                    <div style={{ display: 'flex', alignItems: 'center', gap: '20px', textAlign: 'justify' }}>
                        <h2>Name:</h2>
                        <p>{displayName}</p>
                    </div>
                    <div style={{ display: 'flex', alignItems: 'center', gap: '20px' }}>
                        <h2>Role:</h2>
                        <p>{role}</p>
                    </div>
                    <Box component="form" noValidate sx={{ mt: 3 }}  >
                        <Grid container spacing={2}>
                            <Grid item xs={12} sm={6}>
                                <TextField
                                    name="name"
                                    fullWidth
                                    id="fullName"
                                    label="Name"
                                    value={values.name}
                                    onChange={handleChange("name")}
                                    autoFocus
                                />
                            </Grid>
                            <Grid item xs={12}>
                                <TextField
                                    required
                                    fullWidth
                                    name="password"
                                    label="Current Password"
                                    type="password"
                                    id="currentPassword"
                                    onChange={handleChange("password")}
                                    value={values.password}
                                    autoComplete="new-password"
                                    helperText={error ? "Password too short!" : ""}
                                    error={error}
                                />
                            </Grid>
                            <Grid item xs={12}>
                                <TextField
                                    fullWidth
                                    name="newPassword"
                                    label="New Password"
                                    type="password"
                                    id="newPassword"
                                    onChange={handleChange("newPassword")}
                                    value={values.newPassword}
                                    autoComplete="new-password"
                                />
                            </Grid>
                        </Grid>
                        <Button
                            type="submit"
                            fullWidth
                            variant="contained"
                            sx={{ mt: 3, mb: 2 }}
                            disabled={!isFormValid() || isUpdateLoading}
                            onClick={handleSubmit}
                            noValidate
                        >
                            Update
                        </Button>
                    </Box>
                </div>
                <footer id='footer' style={{ height: '10%', width: '100%', display: 'flex', justifyContent: 'center', gap: '60px', padding: '10px', backgroundColor: '#156082', position: 'fixed', bottom: '0', alignItems: 'center' }}>
                    <LogoutButton></LogoutButton>
                    <Button id="backbutton" startIcon={<FiRotateCcw />} onClick={handleBack} sx={{ borderRadius: 2, width: '120px', height: '60px', color: 'white', textTransform: 'none', fontSize: '18px', '&:hover': { backgroundColor: '#115293' } }}>
                        Back
                    </Button>
                </footer>
                <AlertDialog open={confirmationOpen} onClose={closeDialog}></AlertDialog>

            </div>
        </div>
    );
};

export default UserPage;
