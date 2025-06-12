package com.riot.matesense.datatypes;

import jakarta.persistence.Embeddable;

@Embeddable
    public class Coordinate {
        private Double latitude;
        private Double longitude;

    public Coordinate(Double latitude, Double longitude) {
        this.latitude = latitude;
        this.longitude = longitude;
    }

    public Coordinate() {

    }
    // Constructors, Getters, Setters
    }
