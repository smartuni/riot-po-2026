import Sidebar from './Sidebar';
import InfoBoxes from './InfoBoxes';
import MapView from './MapViewBig';
import HeaderBar from "./HeaderBar";
import MapViewBig from "./MapViewBig";


const MapPage = () => {
    return (
        <div>
            <HeaderBar />
            <div className="container">
                <Sidebar />
                <div className="main">
                    <InfoBoxes />
                    <div className="content">
                        <MapViewBig />
                    </div>
                </div>
            </div>
        </div>
    )
}

export default MapPage