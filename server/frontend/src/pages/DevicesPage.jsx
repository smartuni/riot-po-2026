import AppLayout from '../features/shell/components/AppLayout';
import ComingSoonHero from '../shared/components/ComingSoonHero';
import { SkeletonLine, SkeletonCircle } from '../shared/components/SkeletonLoader';
import '../shared/styles/theme.css';

const DEVICE_CARDS = 5;

const DevicesPage = () => (
  <AppLayout>
      <ComingSoonHero
        icon="📱"
        title="Devices"
        subtitle="Manage and monitor connected IoT devices and sensors"
      />
      <div className="coming-soon-grid">
        {Array.from({ length: DEVICE_CARDS }).map((_, i) => (
          <div key={i} className="skeleton-card">
            <SkeletonCircle size={12} />
            <SkeletonLine width="60%" />
            <SkeletonLine width="90%" />
            <SkeletonLine width="70%" />
          </div>
        ))}
      </div>
    </AppLayout>
);

export default DevicesPage;
