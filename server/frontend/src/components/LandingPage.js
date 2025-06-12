import Button from '@mui/material/Button';
import Box from '@mui/material/Box';
import Grid2 from '@mui/material/Grid';
import Typography from '@mui/material/Typography';
import { Container } from '@mui/material';
import { useNavigate } from 'react-router-dom';
import { IoAccessibilityOutline } from "react-icons/io5";


const LandingPage = () => {
  const navigate = useNavigate();


  return (
    <Container component="main" maxWidth="lg" sx={{ mt: 6, mb: 6 }}>
      <Grid2 
        container 
        spacing={4} 
        sx={{ 
          justifyContent: 'center', 
          alignItems: 'center', 
          minHeight: '70vh', 
          flexWrap: { xs: 'wrap', md: 'nowrap' } 
        }}
      >
        {/* Decorative background circle top-left */}

        {/* Decorative background circle top-left */}
<Box
  sx={{
    position: 'absolute',
    top: '10%',
    left: '10%',
    width: 300,
    height: 300,
    backgroundColor: 'primary.light',
    borderRadius: '50%',
    opacity: 0.8,
    zIndex: 0,
  }}
/>


{/* Decorative Icon */}
<Box
        sx={{
          position: 'absolute',
          down: '100%',
          left: 50,
          zIndex: 0,
          opacity: 0.8,
          fontSize: 240,
          transform: 'rotate(15deg)',
          color: 'primary.main',
          pointerEvents: 'none',
        }}
      >
        <IoAccessibilityOutline />
      </Box>

      {/* Decorative Icon */}
      <Box
        sx={{
          position: 'absolute',
          top: 40,
          right: -20,
          zIndex: 0,
          opacity: 0.8,
          fontSize: 240,
          transform: 'rotate(-15deg)',
          color: 'primary.main',
          pointerEvents: 'none',
        }}
      >
        <IoAccessibilityOutline />
      </Box>

      {/* Decorative Circle */}
      <Box
        sx={{
          position: 'absolute',
          bottom: 100,
          left: -80,
          width: 200,
          height: 200,
          borderRadius: '50%',
          background: 'rgba(255, 0, 0, 0.1)',
          zIndex: 0,
        }}
      />


{/* Decorative background circle bottom-right */}
<Box
  sx={{
    position: 'absolute',
    bottom: '20%',
    right: '20%',
    width: 300,
    height: 300,
    backgroundColor: 'secondary.light',
    borderRadius: '50%',
    opacity: 0.8,
    zIndex: 0,
  }}
/>

<Box
  sx={{
    position: 'absolute',
    top: -40,
    left: -40,
    width: 120,
    height: 120,
    backgroundColor: 'primary.light',
    borderRadius: '50%',
    opacity: 0.2,
    zIndex: 0,
  }}
/>

{/* Decorative background circle bottom-right */}
<Box
  sx={{
    position: 'absolute',
    bottom: -40,
    right: -40,
    width: 150,
    height: 150,
    backgroundColor: 'secondary.light',
    borderRadius: '50%',
    opacity: 0.2,
    zIndex: 0,
  }}
/>


        
        {/* Square box with content */}
        <Box
          sx={{
            position: 'relative',
            border: '2px solid',
            borderColor: 'primary.main',
            borderRadius: 4,
            p: 4,
            maxWidth: 600,
            width: '100%',
            boxShadow: 4,
            backgroundColor: 'background.paper',
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            textAlign: 'center',
            overflow: 'hidden',
          }}
        >
          {/* Angled corner ribbon */}
          <Box
            sx={{
              position: 'absolute',
              top: 40,
              right: -50,
              transform: 'rotate(50deg)',
              backgroundColor: 'red',
              color: 'white',
              fontWeight: 'bold',
              fontSize: '1rem',
              px: 4,
              py: 1,
              boxShadow: 2,
              whiteSpace: 'nowrap',
            }}
          >
            Powered by RIOT
          </Box>

          

          <Typography variant="h2" component="h1" gutterBottom>
            <Box component="span" sx={{ color: 'primary.main' }}>Sense Mate</Box> Dashboard
          </Typography>

          <Typography variant="h6" sx={{ mb: 3, color: 'text.secondary', fontWeight: 500 }}>
            A Project by <Box component="span" sx={{ color: 'red', fontWeight: 700 }}>RIOT</Box>
          </Typography>

          <Typography variant="body1" sx={{ mb: 4, lineHeight: 1.5, color: 'text.secondary' }}>
            Sense Mate is an intelligent dashboard designed to support real-time flood monitoring and disaster management. Developed as part of our university project, it aims to help local authorities efficiently oversee Hamburg’s critical flood infrastructure.
          </Typography>

          <Typography variant="h5" sx={{ fontWeight: 600, color: 'primary.main', mb: 1 }}>
            Our Mission
          </Typography>
          <Typography variant="body1" sx={{ mb: 4, lineHeight: 1.5, color: 'text.secondary' }}>
            To provide a smart, reliable, and user-friendly platform that enhances urban resilience through the power of IoT (Internet of Things) and data-driven insights.
          </Typography>

          <Box sx={{ display: 'flex', gap: 2, width: '100%', justifyContent: 'center' }}>
            <Button
              variant="contained"
              color="primary"
              onClick={() => navigate('/login')}
              sx={{ borderRadius: 20, flex: 1, maxWidth: 160 }}
            >
              Login
            </Button>
            <Button
              variant="contained"
              color="primary"
              onClick={() => navigate('/register')}
              sx={{ borderRadius: 20, flex: 1, maxWidth: 160 }}
            >
              Sign Up
            </Button>
          </Box>
        </Box>
      </Grid2>

      {/* Team info at the bottom */}
      <Box sx={{ mt: 8, textAlign: 'center', color: 'text.secondary' }}>
        <Typography variant="body1" sx={{ maxWidth: 600, mx: 'auto', mb: 1 }}>
          We are Team RIOT – a group of passionate university students focused on solving real-world problems through innovative technology. Our name stands for Reliable IoT Team, and we believe in building systems that matter.
        </Typography>
      </Box>
    </Container>
  );
}

export default LandingPage;
