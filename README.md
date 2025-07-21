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

## **3. Implementation: A Phased Approach**

We will now construct the CI/CD system methodically. Follow these phases in order, as each step provides a necessary component for the next.

### **Phase 0: Preparing the Target Repository on GitHub**

First, we need a target codebase on GitHub that our pipeline will build and test. If you have an existing project, you may use that. If not, this boilerplate includes a sample C project that you can use.

**To set up the sample project:**

1.  **Create a New GitHub Repository:** Go to your GitHub account and create a new, empty repository. For this guide, we'll assume it's named `my-c-project`.

2.  **Upload the Sample Code:** On your local machine, navigate to the `sample-project/` directory included in this boilerplate. Initialize it as a Git repository and push its contents to the new repository you just created on GitHub.

    ```sh
    # Navigate into the sample project directory
    cd sample-project

    # Initialize a new Git repository
    git init -b main
    git add .
    git commit -m "Initial commit of sample project"

    # Add your new GitHub repository as the remote origin
    # Replace <YOUR_USERNAME> and <YOUR_REPO_NAME> accordingly
    git remote add origin https://github.com/<YOUR_USERNAME>/<YOUR_REPO_NAME>.git

    # Push the code to GitHub
    git push -u origin main
    ```

You now have a live project on GitHub ready to be integrated with our CI/CD system.

### **Phase 1: Provisioning the GitLab Project Hub**

This GitLab project will not store code. It will serve as our CI/CD orchestrator, managing secrets and pipeline execution.

1.  **Create a Blank GitLab Project:**
    *   On your GitLab dashboard, click **Create new project** > **Create blank project**.
    *   Give it a name (e.g., `ci-cd-orchestrator`).
    *   **Important:** Ensure the "Initialize repository with a README" box is **unchecked**.
    *   Click **Create project**.

2.  **Retrieve the Runner Registration Token:**
    *   In your new GitLab project, navigate to **Settings > CI/CD**.
    *   Expand the **Runners** section.
    *   Under the "Project runners" area, find the registration token. Click the copy icon to copy the token (it begins with `glrt-`). Keep this ready for the next phase.

3.  **Securely Store Your GitHub PAT:**
    *   While still in **Settings > CI/CD**, expand the **Variables** section.
    *   Click **Add variable**.
    *   Fill in the form:
        *   **Key:** `GITHUB_PAT`
        *   **Value:** Paste your GitHub Personal Access Token that you created during the prerequisites.
        *   **Flags:** Check the **Mask variable** box to prevent the token from being exposed in job logs.
    *   Click **Add variable**.

### **Phase 2: Commissioning the Self-Hosted Runner**

Now, we will set up the dedicated build agent on your runner machine.

1.  **Prepare the Runner's Directory:**
    *   On your runner machine, create a directory to house the runner's configuration. This guide uses a `runner-setup/` directory. You can find a template for this in the boilerplate.
        ```sh
        mkdir -p runner-setup/config
        cd runner-setup
        ```

2.  **Define the Runner Environment:**
    *   Create a `docker-compose.yml` file inside `runner-setup/` with the following content:
        ```yaml
        version: '3.8'
        services:
          gitlab-runner:
            image: gitlab/gitlab-runner:latest
            restart: always
            volumes:
              - ./config:/etc/gitlab-runner
              - /var/run/docker.sock:/var/run/docker.sock
        ```

3.  **Register the Runner:**
    *   From within the `runner-setup` directory, run the registration command:
        ```sh
        docker-compose run --rm gitlab-runner register
        ```
    *   Answer the prompts precisely, using the registration token from Phase 1. The `tags` value is critical.
        ```text
        Enter the GitLab instance URL: https://gitlab.com/
        Enter the registration token: <PASTE_YOUR_GLRT_TOKEN_HERE>
        Enter a description for the runner: my-dedicated-builder
        Enter tags for the runner (comma-separated): c-builder,linux
        Enter an executor: docker
        Enter the default Docker image: gcc:latest
        ```

4.  **CRITICAL - Verify the Configuration:**
    *   After registration, **open the `config/config.toml` file**. Confirm it is **not empty**. An empty file indicates a failed registration and will cause your pipelines to get "stuck." If it is empty, delete it and repeat the registration step.

5.  **Launch the Runner:**
    *   Start the runner as a background service:
        ```sh
        docker-compose up -d
        ```    *   Verify its status in your GitLab project under **Settings > CI/CD > Runners**. It should appear with a green online indicator.

### **Phase 3: Authoring the Pipeline Configuration**

This file defines the instructions for our CI/CD pipeline.

1.  **Create the File:** In your **local clone of your GitHub project** (`my-c-project`), create a new file named `.gitlab-ci.yml`.

2.  **Add the Pipeline Definition:** Paste the following content into the file. This configuration uses our "On-Demand Clone" strategy.

    ```yaml
    # This pipeline is designed for the GitLab Free Tier workaround.
    # It fetches the code directly from GitHub in the first step.

    stages:
      - setup
      - build
      - test

    variables:
      # Replace with your GitHub username and repository name.
      GITHUB_REPO: "https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git"
      REPO_DIR: "YOUR_REPO_NAME"

    clone_repo:
      stage: setup
      tags:
        - c-builder # Ensures this job runs on our dedicated runner.
      script:
        - echo "Cloning repository from GitHub..."
        # Use the GITHUB_PAT variable for authentication. --depth 1 speeds up the clone.
        - git clone --depth 1 "https://oauth2:${GITHUB_PAT}@${GITHUB_REPO#https://}"
      artifacts:
        # Pass the entire cloned repository to the next stages.
        paths:
          - $REPO_DIR/
        expire_in: 1 hour

    build_job:
      stage: build
      tags:
        - c-builder
      needs: [clone_repo]
      script:
        - echo "Compiling the code..."
        - cd $REPO_DIR
        - make all
      artifacts:
        paths:
          - $REPO_DIR/test_runner
        expire_in: 1 hour

    test_job:
      stage: test
      tags:
        - c-builder
      needs: [build_job]
      script:
        - echo "Running tests..."
        - cd $REPO_DIR
        - chmod +x test_runner
        - ./test_runner
    ```

3.  **Customize Variables:** In the `variables` section, replace `YOUR_USERNAME` and `YOUR_REPO_NAME` with your actual GitHub username and repository name. Save the file, but **do not commit or push it yet.**

### **Phase 4: Establishing the GitLab Baseline & Trigger**

Before we can receive triggers from GitHub, our GitLab project needs two things: a default branch and its own copy of the CI/CD instructions.

1.  **Create the `main` Branch in GitLab:**
    *   In your GitLab `ci-cd-orchestrator` project, there should be a button to **Add README**. Click it.
    *   Accept the default content and **Commit changes**. This single action creates the `main` branch, which is required for our pipeline trigger to work.

2.  **Add the CI/CD Configuration to GitLab:**
    *   On the main page of your GitLab project, click the `+` icon > **New file**.
    *   Set the **File name** to `.gitlab-ci.yml`.
    *   In the content area, paste the entire contents of the `.gitlab-ci.yml` file you authored in Phase 3.
    *   **Commit changes**.

3.  **Create a Pipeline Trigger Token:**
    *   In GitLab, navigate to **Settings > CI/CD** and expand **Pipeline triggers**.
    *   Click **Add trigger**.
    *   A token and a template URL will be created. Copy the **token** value.

4.  **Construct the Webhook URL:**
    *   GitLab provides a template URL like:
        `https://gitlab.com/api/v4/projects/PROJECT_ID/ref/REF_NAME/trigger/pipeline?token=TOKEN`
    *   Replace the placeholders to create your final URL:
        *   Replace `REF_NAME` with `main`.
        *   Replace `TOKEN` with the trigger token you just copied.

### **Phase 5: Finalizing the Connection**

The final step is to create the webhook in GitHub to send notifications to our GitLab project.

1.  **Create the Webhook:**
    *   Navigate to your project repository on **GitHub**.
    *   Go to **Settings > Webhooks** and click **Add webhook**.
    *   **Payload URL:** Paste the final, constructed URL from the previous step.
    *   **Content type:** This is a critical step. Change the value to **`application/x-www-form-urlencoded`**.
    *   Leave the other settings as default.
    *   Click **Add webhook**.

2.  **Test the Webhook:**
    *   After adding the webhook, stay on the page and click on the **Recent Deliveries** tab.
    *   You should see the first delivery. If it has a red error icon, click **Redeliver**.
    *   A successful delivery will have a green checkmark and a **`Response 201`** status code.

3.  **Launch Your First Pipeline:**
    *   You are now ready. On your local machine, commit and push the `.gitlab-ci.yml` file to your GitHub repository.
        ```sh
        # Navigate to your local clone of the GitHub project
        git add .gitlab-ci.yml
        git commit -m "feat: Add GitLab CI pipeline configuration"
        git push
        ```
    *   This push will trigger the webhook, which will start your first pipeline run. Navigate to **Build > Pipelines** in your GitLab project to watch it execute.
