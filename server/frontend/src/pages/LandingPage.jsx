import { Link } from 'react-router-dom';

const LandingPage = () => {
  return (
    <>
      {/* Header */}
      <header className="landing-header">
        <div className="landing-logo">
          <div className="logo-icon">SM</div>
          <span>SenseMate</span>
        </div>
        <nav className="landing-nav">
          <Link to="/login" className="btn btn-ghost-light">Login</Link>
          <Link to="/register" className="btn btn-white">Register</Link>
        </nav>
      </header>

      {/* Hero */}
      <section className="hero">
        <h1>SenseMate</h1>
        <p>Intelligent Flood Gate Monitoring — Real-time control, smart automation, and full visibility over your water infrastructure.</p>
        <div className="hero-buttons">
          <Link to="/register" className="btn btn-hero-primary btn-lg">Get Started</Link>
          <Link to="/dashboard-guest" className="btn btn-hero-ghost btn-lg">View Demo</Link>
        </div>
        <div className="wave-divider">
          <svg viewBox="0 0 1440 60" preserveAspectRatio="none" xmlns="http://www.w3.org/2000/svg">
            <path d="M0,30 C360,60 720,0 1080,30 C1260,45 1380,20 1440,30 L1440,60 L0,60 Z" fill="#ffffff"/>
          </svg>
        </div>
      </section>

      {/* Features */}
      <section className="features">
        <h2>Built for Water Infrastructure</h2>
        <div className="features-grid">
          <div className="feature-card">
            <div className="feature-icon">📊</div>
            <h3>Real-Time Monitoring</h3>
            <p>Track gate status, water levels, and system health with live dashboards and instant alerts.</p>
          </div>
          <div className="feature-card">
            <div className="feature-icon">⚡</div>
            <h3>Smart Automation</h3>
            <p>Automate gate operations with rule-based triggers, scheduled actions, and intelligent flood response.</p>
          </div>
          <div className="feature-card">
            <div className="feature-icon">🎮</div>
            <h3>Full Control</h3>
            <p>Request gate changes, manage access, and maintain a complete audit trail of every action.</p>
          </div>
        </div>
      </section>

      {/* How It Works */}
      <section className="how-it-works">
        <h2>How It Works</h2>
        <div className="steps-grid">
          <div className="step">
            <div className="step-number">1</div>
            <h3>Connect Your Gates</h3>
            <p>Register flood gates and sensors. SenseMate automatically discovers and maps your infrastructure.</p>
          </div>
          <div className="step">
            <div className="step-number">2</div>
            <h3>Monitor in Real-Time</h3>
            <p>Live dashboards show gate status, water levels, and system health — updated every second.</p>
          </div>
          <div className="step">
            <div className="step-number">3</div>
            <h3>Act with Confidence</h3>
            <p>Request changes, automate responses, and maintain full control over every gate in your network.</p>
          </div>
        </div>
      </section>

      {/* Our Mission */}
      <section className="mission-section">
        <h2>Our Mission</h2>
        <p>SenseMate exists to protect communities from flood risk through intelligent, accessible gate monitoring. We believe real-time data and smart automation should be available to every water authority — not just those with legacy SCADA systems. Our platform puts reliable flood control in the hands of the people who need it most.</p>
      </section>

      {/* CTA Section */}
      <section className="cta-section">
        <h2>Ready to take control?</h2>
        <p>Join the operators who trust SenseMate for flood gate monitoring.</p>
        <Link to="/register" className="btn btn-primary" style={{ padding: '14px 32px', fontSize: '16px' }}>Get Started Free</Link>
      </section>
    </>
  );
};

export default LandingPage;
