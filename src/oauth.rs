use dotenv::dotenv;
use reqwest::{header, Client};
use base64::prelude::*;
use std::env;
use std::fs::File;
use std::io::Write;
use reqwest::header::{AUTHORIZATION, CONTENT_TYPE};
use serde::{Deserialize, Serialize};

#[derive(Deserialize, Serialize, Debug)]
struct SpotifyTokenResponse {
    access_token: String,
    token_type: String,
    expires_in: u32,
    scope: String,
    expires_at: u64,
}

pub fn get_credentials() -> (String, String) {
    dotenv().ok();
    let client_id = env::var("CLIENT_ID").expect("Missing CLIENT_ID environment variable.");
    let client_secret = env::var("CLIENT_SECRET").expect("Missing CLIENT_SECRET environment variable.");

    return (client_id, client_secret)
}

pub async fn reload_credentials() -> Result<(), Box<dyn std::error::Error>> {
    dotenv::from_path("../.env").ok();

    // Variables pour client_id et client_secret
    let client_id = env::var("CLIENT_ID").expect("CLIENT_ID not found");;
    let client_secret = env::var("CLIENT_SECRET").expect("CLIENT_SECRET not found");

    // println!("{}:{}", client_id, client_secret);

    // Encodage en base64
    let credentials = format!("{}:{}", client_id, client_secret);

    let encoded_credentials = BASE64_STANDARD.encode(credentials);

    // URL de l'API Spotify
    let url = "https://accounts.spotify.com/api/token";

    // Requête POST avec reqwest
    let client = reqwest::Client::new();
    let response = client
        .post(url)
        .header(AUTHORIZATION, format!("Basic {}", encoded_credentials))
        .header(CONTENT_TYPE, "application/x-www-form-urlencoded")
        .form(&[("grant_type", "client_credentials")])
        .send()
        .await?;

    // Vérification de la réponse
    if response.status().is_success() {
        let auth_response: SpotifyTokenResponse = response.json().await?;
        println!("Token d'accès : {}", auth_response.access_token);
    } else {
        eprintln!("Erreur : {}", response.status());
    }

    Ok(())
}
