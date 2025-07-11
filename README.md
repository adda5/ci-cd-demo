# **Practical CI/CD: A Guide to Hybrid GitHub and GitLab Workflows**

## **1. Introduction & Architecture**

This repository provides a boilerplate template and a comprehensive guide for establishing a professional CI/CD workflow. Specifically, it demonstrates how to host your source code on GitHub while leveraging the power and flexibility of GitLab's CI/CD engine with a private, self-hosted runner.

This guide is designed for engineers and teams who wish to implement a robust, production-ready pipeline. The entire process can be completed in approximately one hour and, importantly, operates entirely within the **GitLab Free Tier**. We will achieve this by manually implementing a workflow that provides the functionality of paid premium features, offering a cost-effective and powerful solution.

#### **The "On-Demand Clone" Architecture**

The core challenge of using the GitLab Free Tier for an external repository is its lack of "pull mirroring," a premium feature that automatically syncs code from GitHub. To overcome this, we employ a common industry pattern known as the "On-Demand Clone."

The data flow is simple, secure, and effective:

1.  **Commit & Push:** A developer commits code and pushes it to the primary repository on **GitHub**.
2.  **Webhook Notification:** The push event immediately fires a **Webhook**, sending a secure, one-way notification to GitLab.
3.  **Pipeline Trigger:** The **GitLab** project, acting as a dedicated CI/CD orchestrator, receives this notification and initiates a new pipeline.
4.  **Job Dispatch:** GitLab's orchestrator assigns the first job in the pipeline to our registered **Self-Hosted Runner**.
5.  **On-Demand Clone:** The runner's first task is to execute a script that authenticates with GitHub (using a secure token) and clones the project source code into its temporary workspace.
6.  **Build & Test:** Subsequent jobs in the pipeline then operate on this locally cloned code to perform compilation, testing, and other user-defined actions.

This architecture maintains a clean separation of concerns: GitHub manages the source code, while GitLab manages the CI/CD orchestration.

## **2. Prerequisites: The Pre-Flight Checklist**

Before beginning the implementation, ensure you have the following accounts, tools, and credentials prepared. This checklist will help guarantee a smooth setup process.

1.  **GitHub Account**
    *   You must have an active GitHub account where you can create and manage repositories.

2.  **GitHub Personal Access Token (PAT)**
    *   This token will be used as a secure password to allow our GitLab runner to authenticate with GitHub and clone your repository's source code.
    *   **To create one:**
        1.  Navigate to your GitHub **Settings** > **Developer settings** > **Personal access tokens** > **Tokens (classic)**.
        2.  Click **Generate new token (classic)**.
        3.  Give it a descriptive **Note** (e.g., `gitlab-ci-runner-token`).
        4.  Set an **Expiration** date (e.g., 30 or 90 days).
        5.  Under **Select scopes**, check the box for **`repo`**. This grants the token the necessary permissions to access your repositories.
        6.  Click **Generate token**.
    *   **Crucial:** Copy the generated token string (it will start with `ghp_`) immediately and store it in a secure, temporary location. You will only be shown this token once.

3.  **GitLab Account**
    *   An active GitLab account is required. The workflow outlined in this guide is specifically designed for the **Free Tier**.

4.  **A Dedicated Runner Environment**
    *   You need a host machine that will act as your dedicated build agent. This can be your local development machine, a spare computer, or a cloud VM.
    *   This machine must have **Docker** and **Docker Compose** installed and fully operational.
